#include "planificador.h"

int ciclosUsados = 0;

/*
void agregarAReady(tripulante unTripu){
	list_add(tripulantesReady, unTripu);
}

*/

void avisarARamCambio(int id,int id_patota,char* estado){
	//CREAR CONEXION
		int socket_cliente;
		socket_cliente= crear_conexion(config_valores.ip_ram,config_valores.puerto_ram);
	//CREAR PAQUETE
		t_paquete* nuevoPaquete = crear_paquete_con_codigo_op(CAMBIAR_ESTADO);
	//AGREGAR A PAQUETE
		agregar_entero_a_paquete(nuevoPaquete, id);
		agregar_a_paquete(nuevoPaquete,estado,(strlen(estado) + 1));

	//ENVIAR PAQUETE
	
		enviar_paquete(nuevoPaquete, socket_cliente);

	// BORRAR PAQUETE
	eliminar_paquete(nuevoPaquete);
	
	int codRam = recibir_operacion(socket_cliente);
	
	if(codRam == OK){
		log_info(logger, "Recibi OK");
	}
	
	// LIBERAR CONEXION
	liberar_conexion(socket_cliente);

}

void planificacionFIFO(){
	log_info(logger,"se inicio la planificacion FIFO");

	int ciclosTotales=0;
	int grado_multitarea = config_valores.multitarea;
	int ejecutando;
	int blockIO;
	int blockIOLoco;
	int tamanioReady;
	int tamanioReadyLoco;
	int sumaLoca;
	int block_sabotaje;
	int block_sabotajeLoco;
	int faltaPorArreglar=0;
	int posicionElegido=0;
	int globalExcec=0;
	int globalExcecLoco=0;
	var_sabotaje = 0;
	while(1){
		//log_info(logger,"Paso un ciclo de cpu");


		sem_wait(&planificacionPausada);
		sem_post(&planificacionPausada);
		if(var_sabotaje==0){

		//lo mas probable es que tenga que poner un semaforo aca y en la consola a la hora de crear
		//un tripu para asegurarme que no se agregue nada a la cola mientras la cuento
		sem_wait(&operacionesTripulantes);
		  ejecutando= list_size(tripulantesExecute);
		  blockIO= list_size(tripulantesBlockIO);

		  tamanioReady=list_size(tripulantesReady);
		sem_post(&operacionesTripulantes);
		if (tamanioReady > 0){
			if (ejecutando<grado_multitarea){
				int i=0;
				while(ejecutando<grado_multitarea && (tamanioReady-i)>0){
					sem_wait(&operacionesTripulantes);
					tripulante* primerTripu= list_get(tripulantesReady,0);
					primerTripu->estado='E';
					list_remove(tripulantesReady,0);
					sem_post(&primerTripu->semaforoTripulante);
					avisarARamCambio(primerTripu->id,primerTripu->id_patota,"E");
					list_add(tripulantesExecute,primerTripu);//POR AHORA EL PANIF ES EL UNICO QUE MANEJA ESTA LISTA
					sem_post(&operacionesTripulantes);
					i++;
					ejecutando++;
					log_info(logger,"Se pasaron algunos tripus de ready a Exec");
				}
			}

		}
		if(ejecutando>0){
			for(int i=0;i<ejecutando;i++){
				sem_wait(&operacionesTripulantes);
				tripulante* tripuEjecutando= list_get(tripulantesExecute,i);
				int pasos;
				int taListo= tripuEjecutando->noTaListo;

				sem_post(&operacionesTripulantes);
				if(taListo==1){

					sem_post(&tripuEjecutando->semaforoTripulante);
					sem_wait(&sincroTripuPlanif);
					 pasos= tripuEjecutando->pasosAdar;
				}
				else {
					sem_wait(&operacionesTripulantes);
					 pasos= tripuEjecutando->pasosAdar;
					sem_post(&operacionesTripulantes);
				}
				log_info(logger,"tripu: %d   x: %d  y: %d  aEjecut: %d ejecut: %d PASOS: %d \n",tripuEjecutando->id,tripuEjecutando->pos_x,tripuEjecutando->pos_y,tripuEjecutando->aEjectuar,tripuEjecutando->ejecutado,pasos);

				if(pasos >0){
					sem_post(&tripuEjecutando->semaforoTripulante);
					sem_wait(&sincroTripuPlanif2);

				} else {
					sem_wait(&operacionesTripulantes);
					int tocaIO= tripuEjecutando->tocaIO;
					int ejecutado= tripuEjecutando->ejecutado;
					int aEjecutar= tripuEjecutando->aEjectuar;
					int ultimaTarea= tripuEjecutando->ultimaTarea;
					sem_post(&operacionesTripulantes);
					if(tocaIO==1){ //NECESITO UN SEMAFORO O ALGO PARA ESTAR 100% SEGURO DE QUE VA A ESTAR SINCRONIZ CON EL TRIPULANTE
					sem_wait(&operacionesTripulantes);
					list_remove(tripulantesExecute,i);
					ejecutando--;
					i--;
					tripuEjecutando->estado='B';
					avisarARamCambio(tripuEjecutando->id,tripuEjecutando->id_patota,"B");
					tripuEjecutando->tocaIO=0;     // TODO ESTO SERIA SUPONIENDO QUE EL IO ES 1 SOLO CICLO
					tripuEjecutando->ejecutado++;
					list_add(tripulantesBlockIO,tripuEjecutando);
					log_info(logger,"Se agrego un tripu a la lista de bloqueados");
					sem_post(&tripuEjecutando->semaforoTripulante);
					sem_post(&operacionesTripulantes);
					 // VER SI DSPS DE 1 DE IO SE DEBE DESBLOQUEAR
				}else if(ejecutado < aEjecutar){
					sem_wait(&operacionesTripulantes);

					tripuEjecutando->ejecutado++;
					if(tripuEjecutando->ejecutado== aEjecutar &&  ultimaTarea==0){


					sem_post(&tripuEjecutando->semaforoTripulante);
					tripuEjecutando->noTaListo=1;

				}

				else if( tripuEjecutando->ejecutado ==  aEjecutar &&  ultimaTarea==1){

					tripuEjecutando->estado='F';
					avisarARamCambio(tripuEjecutando->id,tripuEjecutando->id_patota,"F"); //que lo saque de memoria
					list_remove(tripulantesExecute,i); 
					ejecutando--;
					i--;
					list_add(tripulantesFinalizados,tripuEjecutando);
					sem_post(&tripuEjecutando->semaforoTripulante);
				}
					sem_post(&operacionesTripulantes);
				}
				} 
			}

		}
		if(blockIO>0){
				sem_wait(&operacionesTripulantes);
				tripulante* tripuBlock= list_get(tripulantesBlockIO,0);
				log_info(logger, "TRIPUBLOCK ID: %d AEJEC: %d EJEC: %d",tripuBlock->id, tripuBlock->aEjectuar, tripuBlock->ejecutado);
				tripuBlock->ejecutado ++;
				int ultimaTarea= tripuBlock->ultimaTarea;
				if(tripuBlock->ejecutado == tripuBlock->aEjectuar && ultimaTarea==0){
					tripuBlock->estado='R';
					avisarARamCambio(tripuBlock->id,tripuBlock->id_patota,"R");
					tripuBlock->tocaIO=0;
					list_remove(tripulantesBlockIO,0);
					list_add(tripulantesReady,tripuBlock);
					sem_post(&tripuBlock->semaforoTripulante);
					log_info(logger,"Se paso un tripu a ready");
					blockIO--;
					tripuBlock->noTaListo=1;
				}else if(tripuBlock->ejecutado == tripuBlock->aEjectuar && ultimaTarea==1){
					tripuBlock->estado='F';
					avisarARamCambio(tripuBlock->id,tripuBlock->id_patota,"F");
					list_remove(tripulantesBlockIO,0);
					list_add(tripulantesFinalizados,tripuBlock);
					sem_post(&tripuBlock->semaforoTripulante);
					blockIO--;
				}

				sem_post(&operacionesTripulantes);


		}
		}else if(var_sabotaje==1){
			  faltaPorArreglar= config_valores.dur_sabotaje;
			sem_wait(&operacionesTripulantes);
			globalExcec= list_size(tripulantesExecute);
			globalExcecLoco = globalExcec;
			  blockIO= list_size(tripulantesBlockIO);
			  blockIOLoco = blockIO;
			  block_sabotaje=list_size(tripulantesBlock);
			  block_sabotajeLoco = block_sabotaje;
			  tamanioReady=list_size(tripulantesReady);
			  tamanioReadyLoco = tamanioReady;
			  sumaLoca = tamanioReady+globalExcec;
			sem_post(&operacionesTripulantes);
			for(int i=0;i<globalExcec;i++){
				tripulante* tripuEjecutando=list_get(tripulantesExecute,i);
				tripuEjecutando->estado='B';
				avisarARamCambio(tripuEjecutando->id,tripuEjecutando->id_patota,"B");
				list_remove(tripulantesExecute,i);
				globalExcec--;
				i--;
				list_add(tripulantesBlock,tripuEjecutando);
			}
			for(int i=0;i<tamanioReady;i++){
				tripulante* tripuEjecutando=list_get(tripulantesReady,i);
				tripuEjecutando->estado='B';
				avisarARamCambio(tripuEjecutando->id,tripuEjecutando->id_patota,"B");
				list_remove(tripulantesReady,i);
				tamanioReady--;
				i--;
				list_add(tripulantesBlock,tripuEjecutando);
			}
			for(int i=0;i<sumaLoca;i++){
				tripulante* candidato= list_get(tripulantesBlock,i);
				if(i==0 && candidato->tocaIO!=1){
					posicionElegido=i;
				}
					tripulante * aComparar= list_get(tripulantesBlock,posicionElegido);
					int distanciaComparar= pasosAdar(aComparar->pos_x,aComparar->pos_y,sabotaje_x,sabotaje_y);
					int distCandidato= pasosAdar(candidato->pos_x,candidato->pos_y,sabotaje_x,sabotaje_y);
					if(distanciaComparar>distCandidato){
						posicionElegido=i;

				}
			}
			//while(1){}
			tripulante* elegido= list_get(tripulantesBlock,posicionElegido);
			elegido->elegidoPorElDestino=1; 
			sem_post(&elegido->semaforoTripulante);
			list_remove(tripulantesBlock,posicionElegido);
			list_add(bloqueadosEmergencia,elegido); // TODO SI LA POSICION ES 0 O 1 DSPS DEVOLVERLO A EXCEC
			var_sabotaje=2;

		}else{
			tripulante* elegido= list_get(bloqueadosEmergencia,0);
			sem_wait(&operacionesTripulantes);
			int pasosRestantes=elegido->pasosAdar;
			sem_post(&operacionesTripulantes);
			if(pasosRestantes>0){
				sem_post(&elegido->semaforoTripulante);
				sem_wait(&sincroTripuPlanif);
			}
			else if(pasosRestantes==0 && faltaPorArreglar>0)
			faltaPorArreglar--;

			//sem_post(&elegido->semaforoTripulante);
			if(faltaPorArreglar==0){
				var_sabotaje=0;
				int noEra=0;
					tripulante* elegido= list_get(bloqueadosEmergencia,0);
					sem_post(&elegido->semaforoTripulante);
					elegido->elegidoPorElDestino=0;
					if(posicionElegido<config_valores.multitarea){
						noEra=1;
						list_remove(bloqueadosEmergencia,0);
						elegido->estado='E';
						avisarARamCambio(elegido->id,elegido->id_patota,"E");
						list_add(tripulantesExecute,elegido);
						globalExcecLoco--;
					}
					int tripusBlock=list_size(tripulantesBlock);
					int entro= 0;
					int tripusEmergencia = list_size(bloqueadosEmergencia);
					int iTotales=0;
					int tripus = tripusBlock + tripusEmergencia;
					for(int i=0;i<tripus;i++){
						if(i<globalExcecLoco){
							tripulante* tripu=list_get(tripulantesBlock,i);
							list_remove(tripulantesBlock,i);
							tripus--;
							i--;
							globalExcecLoco--;
							tripu->estado='E';
							avisarARamCambio(tripu->id,tripu->id_patota,"E");
							list_add(tripulantesExecute,tripu);
						}else{
							if(posicionElegido == (iTotales-grado_multitarea+1) && entro==0 && noEra==0){
								tripulante* tripu=list_get(bloqueadosEmergencia,0);
								list_remove(bloqueadosEmergencia,0);
								tripus--;
							    i--;
							    tripu->estado='R';
							    entro=1;
								avisarARamCambio(tripu->id,tripu->id_patota,"R");
								list_add(tripulantesReady,tripu);
							}else{
								tripulante* tripu=list_get(tripulantesBlock,i);
								 list_remove(tripulantesBlock,i);
								 tripus--;
								 i--;
								 tripu->estado='R';
								 avisarARamCambio(tripu->id,tripu->id_patota,"R");
								 list_add(tripulantesReady,tripu);
								
							}
						}
						iTotales++;
					}
				}

			}

		ciclosTotales++;
		sleep(config_valores.tiempo_ciclo_cpu);
	}

}

void planificacionRR(){
	log_info(logger,"se inicio la planificacion RR");
int ciclosTotales=0;
	int grado_multitarea = config_valores.multitarea;
	int ejecutando;
	int blockIO;
	int blockIOLoco;
	int tamanioReady;
	int tamanioReadyLoco;
	int block_sabotaje;
	int block_sabotajeLoco;
	int faltaPorArreglar=0;
	int posicionElegido=0;
	int globalExcec=0;
	int globalExcecLoco=0;
	int sumaLoca;
	var_sabotaje = 0;
	
	

	
	
	while(1){

		sem_wait(&planificacionPausada);
		sem_post(&planificacionPausada);
		if(var_sabotaje==0){

		sem_wait(&operacionesTripulantes);
		  ejecutando= list_size(tripulantesExecute);
		  blockIO= list_size(tripulantesBlockIO);

		  tamanioReady=list_size(tripulantesReady);
		sem_post(&operacionesTripulantes);
		if (tamanioReady > 0){
			if (ejecutando<grado_multitarea){
				int i=0;
				while(ejecutando<grado_multitarea && (tamanioReady-i)>0){
					sem_wait(&operacionesTripulantes);
					tripulante* primerTripu= list_get(tripulantesReady,0);
					sem_post(&operacionesTripulantes);
					if(primerTripu->estado == 'R'){
						sem_wait(&operacionesTripulantes);
						list_remove(tripulantesReady,0);
						primerTripu->estado='E';
						primerTripu->ciclos=0;
						if (primerTripu->noTaListo==1 ){
						sem_post(&primerTripu->semaforoTripulante);}
						avisarARamCambio(primerTripu->id,primerTripu->id_patota,"E");
						list_add(tripulantesExecute,primerTripu);//POR AHORA EL PANIF ES EL UNICO QUE MANEJA ESTA LISTA
						sem_post(&operacionesTripulantes);
						i++;
						ejecutando++;
					}
				}
			}
		}
		if(ejecutando>0){
			for(int i=0;i<ejecutando;i++){
				sem_wait(&operacionesTripulantes);
				tripulante* tripuEjecutando= list_get(tripulantesExecute,i);
				int pasos;
				int taListo= tripuEjecutando->noTaListo;
				sem_post(&operacionesTripulantes);
				if(taListo==1){
					sem_post(&tripuEjecutando->semaforoTripulante);
					sem_wait(&sincroTripuPlanif);
					 pasos= tripuEjecutando->pasosAdar;
				}
				else {
					sem_wait(&operacionesTripulantes);
					 pasos= tripuEjecutando->pasosAdar;
					sem_post(&operacionesTripulantes);
				}
				log_info(logger,"tripu: %d   x: %d  y: %d  aEjecut: %d ejecut: %d PASOS: %d \n",tripuEjecutando->id,tripuEjecutando->pos_x,tripuEjecutando->pos_y,tripuEjecutando->aEjectuar,tripuEjecutando->ejecutado,pasos);

				if(pasos >0){

					tripuEjecutando->ciclos+=1;				 //SUMO UN CILCO UTIL PARA RR
					sem_post(&tripuEjecutando->semaforoTripulante);
					sem_wait(&sincroTripuPlanif2);
					sem_wait(&operacionesTripulantes);
					//tripuEjecutando->ejecutado ++;
					if(tripuEjecutando->ciclos>=config_valores.quantum){
						list_remove(tripulantesExecute,i);
						ejecutando--;
						i--;
						tripuEjecutando->estado='R';
						avisarARamCambio(tripuEjecutando->id,tripuEjecutando->id_patota,"R");
						list_add(tripulantesReady,tripuEjecutando);
						log_info(logger,"se paso un tripu a ready");
					}
					sem_post(&operacionesTripulantes);


				}else {
					//log_info(logger,"tripu: %d   x: %d  y: %d  aEjecut: %d ejecut: %d PASOS: %d \n",tripuEjecutando->id,tripuEjecutando->pos_x,tripuEjecutando->pos_y,tripuEjecutando->aEjectuar,tripuEjecutando->ejecutado,pasos);

					sem_wait(&operacionesTripulantes);
					int tocaIO= tripuEjecutando->tocaIO;
					int ejecutado= tripuEjecutando->ejecutado;
					int aEjecutar= tripuEjecutando->aEjectuar;
					int ultimaTarea= tripuEjecutando->ultimaTarea;
					sem_post(&operacionesTripulantes);

				if(tocaIO==1){ //NECESITO UN SEMAFORO O ALGO PARA ESTAR 100% SEGURO DE QUE VA A ESTAR SINCRONIZ CON EL TRIPULANTE
					log_info(logger,"llegue a meterme en block %d",tripuEjecutando->id);
					sem_wait(&operacionesTripulantes);
					list_remove(tripulantesExecute,i);
					ejecutando--;
					i--;
					tripuEjecutando->estado='B';
					avisarARamCambio(tripuEjecutando->id,tripuEjecutando->id_patota,"B");
					tripuEjecutando->tocaIO=0;     // TODO ESTO SERIA SUPONIENDO QUE EL IO ES 1 SOLO CICLO
					tripuEjecutando->ejecutado++;
					tripuEjecutando->ciclos=0;
					list_add(tripulantesBlockIO,tripuEjecutando);
					sem_post(&tripuEjecutando->semaforoTripulante);
					sem_post(&operacionesTripulantes);
					log_info(logger,"se paso un tripu a block");
					 // VER SI DSPS DE 1 DE IO SE DEBE DESBLOQUEAR

				}else if(ejecutado < aEjecutar){

					sem_wait(&operacionesTripulantes);
					tripuEjecutando->ejecutado++;
					tripuEjecutando->ciclos++;
					if(tripuEjecutando->ciclos>=config_valores.quantum && tripuEjecutando->ejecutado <aEjecutar){
						tripuEjecutando->ciclos=0;
						list_remove(tripulantesExecute,i);
						ejecutando--;
						i--;
						list_add(tripulantesReady,tripuEjecutando);
						log_info(logger,"se paso un tripu a ready");
						tripuEjecutando->estado='R';
						avisarARamCambio(tripuEjecutando->id,tripuEjecutando->id_patota,"R");
					}

					else if(tripuEjecutando->ejecutado == aEjecutar && ultimaTarea==0 &&tripuEjecutando->ciclos<config_valores.quantum){

						tripuEjecutando->noTaListo=1;
					sem_post(&tripuEjecutando->semaforoTripulante);
				}else if(tripuEjecutando->ejecutado == aEjecutar && ultimaTarea==0 &&tripuEjecutando->ciclos>=config_valores.quantum){

					tripuEjecutando->estado='R';
					 list_remove(tripulantesExecute,i);
					 ejecutando--;
					 i--;
				     tripuEjecutando->ciclos=0;
				     avisarARamCambio(tripuEjecutando->id,tripuEjecutando->id_patota,"R");
					 list_add(tripulantesReady,tripuEjecutando);
					 log_info(logger,"se paso un tripu a ready");
					 //sem_post(&tripuEjecutando->semaforoTripulante);
					 tripuEjecutando->noTaListo=1;

				}else if(tripuEjecutando->ejecutado==aEjecutar && ultimaTarea==1){


					tripuEjecutando->estado='F';
					list_remove(tripulantesExecute,i); 
					ejecutando--;
					i--;
					list_add(tripulantesFinalizados,tripuEjecutando);
				     avisarARamCambio(tripuEjecutando->id,tripuEjecutando->id_patota,"F");
					sem_post(&tripuEjecutando->semaforoTripulante);
				}sem_post(&operacionesTripulantes);
				}
				}
			}

		}
		if(blockIO>0){

						sem_wait(&operacionesTripulantes);
						tripulante* tripuBlock= list_get(tripulantesBlockIO,0);
						tripuBlock->ejecutado ++;
						int ultimaTarea= tripuBlock->ultimaTarea;
						log_info(logger, "TRIPUBLOCK ID: %d AEJEC: %d EJEC: %d",tripuBlock->id, tripuBlock->aEjectuar, tripuBlock->ejecutado);
						if(tripuBlock->ejecutado == tripuBlock->aEjectuar && ultimaTarea==0){
							tripuBlock->estado='R';
							avisarARamCambio(tripuBlock->id,tripuBlock->id_patota,"R");
							tripuBlock->tocaIO=0;
							tripuBlock->pasosAdar=0;
							list_remove(tripulantesBlockIO,0);
							list_add(tripulantesReady,tripuBlock);
							sem_post(&tripuBlock->semaforoTripulante);
							log_info(logger,"Se paso un tripu a ready");
							blockIO--;
							tripuBlock->noTaListo=1;
						}else if(tripuBlock->ejecutado == tripuBlock->aEjectuar && ultimaTarea==1){
							tripuBlock->estado='F';
							avisarARamCambio(tripuBlock->id,tripuBlock->id_patota,"F");
							list_remove(tripulantesBlockIO,0);
							list_add(tripulantesFinalizados,tripuBlock);
							sem_post(&tripuBlock->semaforoTripulante);
							blockIO--;
						}

						sem_post(&operacionesTripulantes);

					}
		}else if(var_sabotaje==1){
			  faltaPorArreglar= config_valores.dur_sabotaje;
			sem_wait(&operacionesTripulantes);
			globalExcec= list_size(tripulantesExecute);
			globalExcecLoco=globalExcec;
			  blockIO= list_size(tripulantesBlockIO);
			  blockIOLoco=blockIO;
			  block_sabotaje=list_size(tripulantesBlock);
			  block_sabotajeLoco=block_sabotaje;
			  tamanioReady=list_size(tripulantesReady);
			  tamanioReadyLoco=tamanioReady;
			  sumaLoca=tamanioReady + globalExcec;
			sem_post(&operacionesTripulantes);
			for(int i=0;i<globalExcec;i++){
				tripulante* tripuEjecutando=list_get(tripulantesExecute,i);
				tripuEjecutando->estado='B';
				tripuEjecutando->ciclos=0;
				avisarARamCambio(tripuEjecutando->id,tripuEjecutando->id_patota,"B");
				list_remove(tripulantesExecute,i);
				globalExcec--;
				i--;
				tripuEjecutando->estadoViejo = 'E';

				list_add(tripulantesBlock,tripuEjecutando);
			}
			for(int i=0;i<tamanioReady;i++){
				tripulante* tripuEjecutando=list_get(tripulantesReady,i);
				tripuEjecutando->estado='B';
				tripuEjecutando->ciclos=0;
				tripuEjecutando->estadoViejo = 'R';
				avisarARamCambio(tripuEjecutando->id,tripuEjecutando->id_patota,"B");
				list_remove(tripulantesReady,i);
				tamanioReady--;
				i--;
				list_add(tripulantesBlock,tripuEjecutando);
			}
			for(int i=0;i<sumaLoca;i++){
				tripulante* candidato= list_get(tripulantesBlock,i);
				if(i==0){
					posicionElegido=i;
				}
				else{
					tripulante * aComparar= list_get(tripulantesBlock,posicionElegido);
					int distanciaComparar= pasosAdar(aComparar->pos_x,aComparar->pos_y,sabotaje_x,sabotaje_y);
					int distCandidato= pasosAdar(candidato->pos_x,candidato->pos_y,sabotaje_x,sabotaje_y);
					if(distanciaComparar>distCandidato){
						posicionElegido=i;
					}
				}
			}
			tripulante* elegido= list_get(tripulantesBlock,posicionElegido);
			elegido->elegidoPorElDestino=1; 
			sem_post(&elegido->semaforoTripulante);
			list_remove(tripulantesBlock,posicionElegido);
			list_add(bloqueadosEmergencia,elegido); // TODO SI LA POSICION ES 0 O 1 DSPS DEVOLVERLO A EXCEC
			var_sabotaje=2;

		}else{
			tripulante* elegido= list_get(bloqueadosEmergencia,0);
			sem_wait(&operacionesTripulantes);
			int pasosRestantes=elegido->pasosAdar;
			sem_post(&operacionesTripulantes);
			if(pasosRestantes>0){
				sem_post(&elegido->semaforoTripulante);
				sem_wait(&sincroTripuPlanif);
			}
			else if(pasosRestantes==0 && faltaPorArreglar>0)
			faltaPorArreglar--;

			//sem_post(&elegido->semaforoTripulante);
			if(faltaPorArreglar==0){
				var_sabotaje=0;
				int noEra=0;
					tripulante* elegido= list_get(bloqueadosEmergencia,0);
					sem_post(&elegido->semaforoTripulante);
					elegido->elegidoPorElDestino=0;
					if(posicionElegido<config_valores.multitarea && elegido->estadoViejo== 'E'){
						noEra=1;
						list_remove(bloqueadosEmergencia,0);
						elegido->estado='E';
						avisarARamCambio(elegido->id,elegido->id_patota,"E");
						list_add(tripulantesExecute,elegido);
						globalExcecLoco--;
					}
					int tripusBlock=list_size(tripulantesBlock);
					int entro= 0;
					int tripusEmergencia = list_size(bloqueadosEmergencia);
					int iTotales=0;
					int tripus = tripusBlock + tripusEmergencia;
					for(int i=0;i<tripus;i++){
						if(i<globalExcecLoco && elegido->estadoViejo== 'E'){
							tripulante* tripu=list_get(tripulantesBlock,i);
							list_remove(tripulantesBlock,i);
							tripus--;
							i--;
							globalExcecLoco--;
							tripu->estado='E';
							avisarARamCambio(tripu->id,tripu->id_patota,"E");
							list_add(tripulantesExecute,tripu);
						}else{
							if(posicionElegido == (iTotales-grado_multitarea+1) && entro==0 && noEra==0){
								tripulante* tripu=list_get(bloqueadosEmergencia,0);
								list_remove(bloqueadosEmergencia,0);
								tripus--;
							    i--;
							    tripu->estado='R';
							    entro=1;
								avisarARamCambio(tripu->id,tripu->id_patota,"R");
								list_add(tripulantesReady,tripu);
							}else{
								tripulante* tripu=list_get(tripulantesBlock,i);
								 list_remove(tripulantesBlock,i);
								 tripus--;
								 i--;
								 tripu->estado='R';
								 avisarARamCambio(tripu->id,tripu->id_patota,"R");
								 list_add(tripulantesReady,tripu);
								
							}
						}
						iTotales++;
					}
				}

			}

		ciclosTotales++;
		sleep(config_valores.tiempo_ciclo_cpu);
	}

}





 
