#include "tripulante.h"

void iniciarPatota(int cantidad, int idPatota, int idUltimoTripulanteCreado,char** posiciones){ // VER SI CONOZCO A LA PATOTA POR ID O POR NUMERO DE CREACION
	for(int i=0;i<cantidad;i++){
		char** split = string_split(posiciones[i], "|");
		int posX= atoi(split[0]);
		int posY= atoi(split[1]);
		tripulante* unTripu= calloc(1,sizeof(tripulante));
		sem_t unSemaforo;
		unTripu->pos_x = posX;
		unTripu->pos_y = posY;
		unTripu->id = idUltimoTripulanteCreado;
		unTripu->id_patota = idPatota;
		unTripu->ciclos=0;
		pthread_t procesoTripu;
		unTripu->estado = 'N';
		unTripu->tocaIO=0;
		sem_init(&unSemaforo,0,0);
		unTripu->semaforoTripulante = unSemaforo;
		unTripu->ejecutado=0;
		unTripu->noTaListo=1;
		unTripu->elegidoPorElDestino=0; 
		//CREAR PAQUETE
			t_paquete* nuevoPaquete = crear_paquete_con_codigo_op(INICIAR_TRIPULANTE);
		//AGREGAR A PAQUETE 
			agregar_entero_a_paquete(nuevoPaquete, idUltimoTripulanteCreado); 
			agregar_entero_a_paquete(nuevoPaquete, idPatota);
			agregar_entero_a_paquete(nuevoPaquete, posX);
			agregar_entero_a_paquete(nuevoPaquete, posY);
		//ENVIAR PAQUETE
			int conexion = intentar_enviar_paquete(nuevoPaquete, config_valores.ip_ram, config_valores.puerto_ram);
		// BORRAR PAQUETE
			log_info(logger,"Paquete para iniciar tripulante enviado a mi-ram");
			eliminar_paquete(nuevoPaquete);
		// LIBERAR CONEXION
			//liberar_conexion(conexion);
			t_paquete* paquete_mongo = crear_paquete_con_codigo_op(INICIAR_TRIPULANTE);
			agregar_entero_a_paquete(paquete_mongo, idUltimoTripulanteCreado);
			sem_wait(&operacionesTripulantes);
			int conexion_mongo = intentar_enviar_paquete(paquete_mongo, config_valores.ip_mongo, config_valores.puerto_mongo);
			log_info(logger,"Paquete para iniciar tripulante enviado a imongo");
			int cod_mongo= recibir_operacion(conexion_mongo);


				// BORRAR PAQUETE*/
							eliminar_paquete(paquete_mongo);
						// LIBERAR CONEXION
				liberar_conexion(conexion_mongo);

			sem_post(&operacionesTripulantes);


		idUltimoTripulanteCreado ++;
		sem_wait(&operacionesTripulantes);
		list_add(tripulantesNuevos , unTripu);
		sem_post(&operacionesTripulantes);
		pthread_create(&procesoTripu,NULL,(void*) funcionesTripulantes,(void*)unTripu);
		//unTripu->hilo = procesoTripu;
		pthread_detach(procesoTripu); //PREGUNTAR SI EL HILO NO ARRANCA A EJECUTAR HASTA EL DETACH

	}

}

void funcionesTripulantes (tripulante* tripu){
	sem_wait(&planificacionPausada);
	sem_post(&planificacionPausada);
	sem_wait(&operacionesTripulantes); //lo meto para asegurarme que todos van a llegar en orden al semaforo de planificacion
	//PEDIR A MI RAM LA SIGUIENTE INSTRUCCION
	t_paquete* paqueteRam = crear_paquete_con_codigo_op(ENVIAR_OP);
	//AGREGAR A PAQUETE
	agregar_entero_a_paquete(paqueteRam, tripu->id_patota); // AGREGAMOS ID PATOTA
	agregar_entero_a_paquete(paqueteRam, tripu->id ); // AGREGAMOS ID TRIPU
	//ENVIAR PAQUETE
	int conexionRam = intentar_enviar_paquete(paqueteRam, config_valores.ip_ram, config_valores.puerto_ram);
	int cod_op = recibir_operacion(conexionRam);
	//BORRAR PAQUETE
	eliminar_paquete(paqueteRam);
	if(cod_op==MOVERSE){
		tripu->tocaIO=0;
	}else{
		tripu->tocaIO=1;
	}

	int tamanioLNuevos= list_size(tripulantesNuevos);
	int posicionMiTripu;
	for (int i=0;i<tamanioLNuevos;i++){
		tripulante * tripuAcomparar = list_get(tripulantesNuevos,i);
		if(tripuAcomparar->id==tripu->id){
			posicionMiTripu=i;
		}
	}
	list_remove(tripulantesNuevos,posicionMiTripu);
	tripu->estado = 'R';
	list_add(tripulantesReady , tripu);
	sem_post(&operacionesTripulantes);
	sem_wait(&planificacionPausada);
	sem_post(&planificacionPausada);
	while(1){
		sem_wait(&tripu->semaforoTripulante);
		if(tripu->elegidoPorElDestino==1){
			avisarleAmongo(tripu->id);
			realizarSabotaje((void*) tripu);
		}else break;
	}
	while(1){
	//TERMINAR DE RECIBIR LAS COSAS DE RAM
	int size;
	char *buffer;
					
	int desp = 0;
	//int ciclos = 0; //VER SI NECESITO CICLOS TOTALES PARA CUANDO USO RR

	tripu->pasosAdar=0;
	buffer = recibir_buffer(&size, conexionRam);
	int posXAIr=leer_entero(buffer, &desp);
	int posYAIr=leer_entero(buffer, &desp);
	int cantAEditar=leer_entero(buffer, &desp); //PARAMETRO DE LA TAREA
	int tiempo =leer_entero(buffer, &desp); //TIEMPO DE ESPERA EN BLOCK, A MENOS QUE LA TAREA SEA DE LAS DE DESPLAZ
	sem_wait(&operacionesTripulantes);
	 tripu->pasosAdar= pasosAdar(tripu->pos_x,tripu->pos_y,posXAIr,posYAIr);

	tripu->ultimaTarea= leer_entero(buffer, &desp);
	//log_info(logger,"TRIPU: %d POSXtripu:%d POSYtrip:%d POSXir:%d POSYir:%d CANT:%d TIME:%d",tripu->id,tripu->pos_x, tripu->pos_y,posXAIr,posYAIr,cantAEditar,tiempo);

	tripu->aEjectuar=tiempo+tripu->pasosAdar; // TODO RECORDAR SUMAR 1 EN EL SWITCH CON LOS QUE TENGAN IO
	sem_post(&operacionesTripulantes);
	sem_wait(&tripu->semaforoTripulante);
	sem_wait(&operacionesTripulantes);
	tripu->noTaListo=0;
	sem_post(&sincroTripuPlanif);
	sem_post(&operacionesTripulantes);
	free(buffer);

	// LIBERAR CONEXION
	liberar_conexion(conexionRam);
	while((tripu -> pos_x != posXAIr) || (tripu -> pos_y != posYAIr)) {

		sem_wait(&tripu->semaforoTripulante);// VER SI PUEDE PASAR QUE EL PLANIFICADOR HAGA POST ANTES DE LLEGAR ACA
			if(tripu->elegidoPorElDestino==1){
				avisarleAmongo(tripu->id);
				realizarSabotaje((void*) tripu);
				sem_wait(&operacionesTripulantes);
					tripu->pasosAdar= pasosAdar(tripu->pos_x,tripu->pos_y,posXAIr,posYAIr);
				sem_post(&operacionesTripulantes);
				sem_wait(&tripu->semaforoTripulante);
		}
		sem_post(&sincroTripuPlanif2);
		sem_wait(&operacionesTripulantes);
		int xOrigen=tripu -> pos_x;
		int yOrigen=tripu -> pos_y;
		sem_post(&operacionesTripulantes);
		sem_wait(&operacionesTripulantes); //TODO CREO QUE ESTE SEMAFORO Y SU POST NO SON NECESARIOS, NADIE ADEMAS DEL TRIPU CAMBIA SU POSICION
		if (tripu -> pos_x > posXAIr) {
			tripu -> pos_x = tripu -> pos_x - 1;
		} else if (tripu -> pos_x < posXAIr) {
			tripu -> pos_x = tripu -> pos_x + 1;
		} else if (tripu -> pos_y > posYAIr) {
			tripu -> pos_y = tripu -> pos_y - 1;
		} else if (tripu -> pos_y < posYAIr) {
			tripu -> pos_y = tripu -> pos_y + 1;
		}
		tripu->pasosAdar--;
		tripu->ejecutado++; 
		sem_post(&operacionesTripulantes);
		//CREAR PAQUETE
		t_paquete* paqueteActualizaUbicacion = crear_paquete_con_codigo_op(ACTUALIZAR_UBICACION);

		//sem_wait(&operacionesTripulantes);
		agregar_entero_a_paquete(paqueteActualizaUbicacion, tripu->id);
		agregar_entero_a_paquete(paqueteActualizaUbicacion, tripu->id_patota);
		agregar_entero_a_paquete(paqueteActualizaUbicacion, tripu->pos_x);// TODO no agrego semaforo pq creo que nadie cambia la pos del tripu
		agregar_entero_a_paquete(paqueteActualizaUbicacion, tripu->pos_y);
		//sem_post(&operacionesTripulantes);
		
		conexionRam = intentar_enviar_paquete(paqueteActualizaUbicacion, config_valores.ip_ram, config_valores.puerto_ram);
		
		int codRam = recibir_operacion(conexionRam);
		// LIBERAR CONEXION

				liberar_conexion(conexionRam);
		//BORRAR PAQUETE
				eliminar_paquete(paqueteActualizaUbicacion);
		//CREAR PAQUETE PARA MONGO
		//pthread_mutex_lock(&mensaje_mongo);
		//sleep(1);
		sem_wait(&operacionesTripulantes);
				t_paquete* paqueteActualizaUbicacionMongo = crear_paquete_con_codigo_op(13);
				sem_post(&operacionesTripulantes);
				sem_wait(&operacionesTripulantes);
				int id=tripu->id;
				int x=tripu->pos_x;
				int y=tripu->pos_y;
				sem_post(&operacionesTripulantes);
				sem_wait(&operacionesTripulantes);
				agregar_entero_a_paquete(paqueteActualizaUbicacionMongo, id);
				agregar_entero_a_paquete(paqueteActualizaUbicacionMongo, x);// TODO no agrego semaforo pq creo que nadie cambia la pos del tripu
				agregar_entero_a_paquete(paqueteActualizaUbicacionMongo, y);
				agregar_entero_a_paquete(paqueteActualizaUbicacionMongo, xOrigen);
				agregar_entero_a_paquete(paqueteActualizaUbicacionMongo, yOrigen);
				sem_post(&operacionesTripulantes);
			
				int conexionActualizaUbMongo;

				conexionActualizaUbMongo= crear_conexion(config_valores.ip_mongo, config_valores.puerto_mongo);
				enviar_paquete(paqueteActualizaUbicacionMongo, conexionActualizaUbMongo);
			//
			
				// LIBERAR CONEXION
				int cod_mongo= recibir_operacion(conexionActualizaUbMongo);

				//enviar_codop(OK, cod_mongo);
						liberar_conexion(conexionActualizaUbMongo);

				//BORRAR PAQUETE
				eliminar_paquete(paqueteActualizaUbicacionMongo);
				//pthread_mutex_unlock(&mensaje_mongo);
				//sem_wait(&tripu->semaforoTripulante);
				sem_wait(&planificacionPausada); //ESTO ESTA POR SI VIENE UN SABOTAJE MIENTRAS ME MUEVO
				sem_post(&planificacionPausada); // PREGUNTAR SI TA BN O NO ES NECESARIO
		/*if(strcmp(config_valores.algoritmo,"RR")==0){
			int ciclosTotales= tripu->ciclos;
			if(ciclosTotales == config_valores.quantum){
				sem_wait(&operacionesTripulantes);
				tripu->ciclos=0;
				sem_post(&operacionesTripulantes);
				sem_wait(&tripu->semaforoTripulante);
			}
		}*/

	}
	sem_wait(&planificacionPausada);
	sem_post(&planificacionPausada);

	switch(cod_op)
		{
		case GENERAR_OXIGENO:

			sem_wait(&operacionesTripulantes);
			tripu->aEjectuar++;
			sem_post(&operacionesTripulantes);
			generarOxigeno(cantAEditar,tripu->id,tripu->id_patota);
			log_info(logger,"Llegue a generar oxigeno");
			while(1){
				sem_wait(&tripu->semaforoTripulante);
				if(tripu->elegidoPorElDestino==1){
					avisarleAmongo(tripu->id);
					realizarSabotaje2((void*) tripu);
				}else break;
			}
			sem_wait(&tripu->semaforoTripulante);

		
				finOxigeno(tripu->id);
			
			log_info(logger,"termine de generar oxigeno");
			break;
		case CONSUMIR_OXIGENO:
			sem_wait(&operacionesTripulantes);
			tripu->aEjectuar++;
			sem_post(&operacionesTripulantes);
			consumirOxigeno(cantAEditar,tripu->id,tripu->id_patota);
			log_info(logger,"Llegue a consumir oxigeno");
			while(1){
				sem_wait(&tripu->semaforoTripulante);
				if(tripu->elegidoPorElDestino==1){
					avisarleAmongo(tripu->id);
					realizarSabotaje2((void*) tripu);
				}else break;
			}
			sem_wait(&tripu->semaforoTripulante);

				finConsumirOxigeno(tripu->id);
				log_info(logger,"termine de consumir oxigeno");
			break;
		case GENERAR_COMIDA:
			sem_wait(&operacionesTripulantes);
			tripu->aEjectuar++;
			sem_post(&operacionesTripulantes);
			generarComida(cantAEditar,tripu->id,tripu->id_patota);
			log_info(logger,"Llegue a generar comida");
			while(1){
				sem_wait(&tripu->semaforoTripulante);
				if(tripu->elegidoPorElDestino==1){
					avisarleAmongo(tripu->id);
					realizarSabotaje2((void*) tripu);
				}else break;
			}
			sem_wait(&tripu->semaforoTripulante);

				finComida(tripu->id);
				log_info(logger,"termine de generar comida");
			break;	
		case CONSUMIR_COMIDA:
			sem_wait(&operacionesTripulantes);
			tripu->aEjectuar++;
			sem_post(&operacionesTripulantes);
			consumirComida(cantAEditar,tripu->id,tripu->id_patota);
			log_info(logger,"Llegue a consumir comida");
			while(1){
				sem_wait(&tripu->semaforoTripulante);
				if(tripu->elegidoPorElDestino==1){
					avisarleAmongo(tripu->id);
					realizarSabotaje2((void*) tripu);
				}else break;
			}
			sem_wait(&tripu->semaforoTripulante);
			log_info(logger,"termine de consumir comida");
				finConsumircomida(tripu->id);
			break;
		case GENERAR_BASURA:
			sem_wait(&operacionesTripulantes);
			tripu->aEjectuar++;
			sem_post(&operacionesTripulantes);
			generarBasura(cantAEditar,tripu->id,tripu->id_patota);
			log_info(logger,"Llegue a generar basura");
			while(1){
				sem_wait(&tripu->semaforoTripulante);
				if(tripu->elegidoPorElDestino==1){
					avisarleAmongo(tripu->id);
					realizarSabotaje2((void*) tripu);
				}else break;
			}
			sem_wait(&tripu->semaforoTripulante);

				finBasura(tripu->id);
				log_info(logger,"termine de generar basura");
			break;
		case DESCARTAR_BASURA:
			sem_wait(&operacionesTripulantes);
			tripu->aEjectuar++;
			sem_post(&operacionesTripulantes);
			descartarBasura(cantAEditar,tripu->id,tripu->id_patota);
			log_info(logger,"Llegue a descartar basura");
			while(1){
				sem_wait(&tripu->semaforoTripulante);
				if(tripu->elegidoPorElDestino==1){
					avisarleAmongo(tripu->id);
					realizarSabotaje2((void*) tripu);
				}else break;
			}
				sem_wait(&tripu->semaforoTripulante);
				finConsumirBasura(tripu->id);
				log_info(logger,"termine de descartar basura");

			break;
		case MOVERSE: //PONELE QUE LE PONEMOS ESE NOMBRE QCYO 
				//inicioTarea(tripu->id,tripu->id_patota);

				inicioMongo(tripu->id );
				log_info(logger,"Llegue a tarea no definida");
				//sem_wait(&tripu->semaforoTripulante); //SE SUPONE QUE NOS MOVIMOS ANTES ASI QUE ESPERAMOS ALEGREMENTE 
				while(1){
					sem_wait(&tripu->semaforoTripulante);
					if(tripu->elegidoPorElDestino==1){
						avisarleAmongo(tripu->id);	
						realizarSabotaje2((void*) tripu);			
					}else break;
				}
				/*if(tripu->elegidoPorElDestino==1){
				avisarleAmongo(tripu->id);	
				realizarSabotaje2((void*) tripu);
				sem_wait(&tripu->semaforoTripulante);
				}
				if(tripu->elegidoPorElDestino==1){
					avisarleAmongo(tripu->id);
					realizarSabotaje2((void*) tripu);
				sem_wait(&tripu->semaforoTripulante);
				}
				if(tripu->elegidoPorElDestino==1){ 
					avisarleAmongo(tripu->id);      //DEBERIA HACERLO DE OTRA FORMA, PERO ANTE LA FALTA DE IDEAS ESTO ANDA
					realizarSabotaje2((void*) tripu);  		// SUPONIENDO QUE NO VOY A TENER LA MALA SUERTE DE QUE EL TRIPU HAGA 3 SABOTAJES AL HILO
				sem_wait(&tripu->semaforoTripulante);
				}*/
				log_info(logger,"termine tarea no definida");
				finMongo(tripu->id);
				break;
		case -1:
			/*log_error(logger, "el cliente se desconecto. Terminando servidor");
			return EXIT_FAILURE;*/
		default: //ES LA DE MOVERSE :D
			
			break;
		}
	sem_wait(&planificacionPausada);  
	sem_post(&planificacionPausada);  
	sem_wait(&operacionesTripulantes);
	tripu->ejecutado=0;
		if(tripu->estado=='F'){
			log_info(logger,"el tripulante termino todas sus tareas \n");
			sem_post(&operacionesTripulantes);
			//avisarARamCambio(tripu->id,tripu->id_patota,"F");
			return; // VER SI EL TRIPU DEJA DE EJECTUAR O HAY QUE USAR EXIT O QUE ONDA
		}
		t_paquete* paqueteRam1 = crear_paquete_con_codigo_op(ENVIAR_OP);
		//AGREGAR A PAQUETE
		agregar_entero_a_paquete(paqueteRam1, tripu->id_patota); // AGREGAMOS ID PATOTA
		agregar_entero_a_paquete(paqueteRam1, tripu->id ); // AGREGAMOS ID TRIPU
		//ENVIAR PAQUETE
		conexionRam = intentar_enviar_paquete(paqueteRam1, config_valores.ip_ram, config_valores.puerto_ram); //CONEXION RAM ESTA DEFINIDA PERO LA LIBERO, TENGO QUE VOLVER A DEFINIRLA?
		cod_op = recibir_operacion(conexionRam);
		//BORRAR PAQUETE
		eliminar_paquete(paqueteRam1);
		if(cod_op==MOVERSE){
				tripu->tocaIO=0;
			}else{
				tripu->tocaIO=1;
			}
			char estado=tripu->estado;
			sem_post(&operacionesTripulantes);
			if (estado=='R'){
				tripu->ciclos=0;
				sem_wait(&tripu->semaforoTripulante);
				tripu->estado= 'E';
			}


	}

}

void avisarleAmongo(int id){
	t_paquete* paqueteMongo = crear_paquete_con_codigo_op(CORRE_SABOTAJE);
			//AGREGAR A PAQUETE
			agregar_entero_a_paquete(paqueteMongo,id);
			int conexionMongo = intentar_enviar_paquete(paqueteMongo, config_valores.ip_mongo, config_valores.puerto_mongo);

			eliminar_paquete(paqueteMongo);
			int codOp = recibir_operacion(conexionMongo);
			liberar_conexion(conexionMongo);
			return;
}


int pasosAdar(int x,int y,int xIr, int yIr){
	int difX,difY,total;
	if (x>xIr){
		difX=x-xIr;
	}else{
		difX=xIr-x;
	}
	if(y>yIr){
		difY=y-yIr;
	}else{
		difY=yIr-y;
	}
	total= difY+difX;
	return total;

}

void realizarSabotaje(tripulante* tripu){
	sem_wait(&operacionesTripulantes);
	int posXAntesDelSabotaje = tripu -> pos_x;
	int posYAntesDelSabotaje = tripu -> pos_y;
	tripu->pasosAdar= pasosAdar(tripu->pos_x,tripu->pos_y,sabotaje_x,sabotaje_y);
	sem_post(&operacionesTripulantes);
		while((tripu -> pos_x != sabotaje_x) || (tripu -> pos_y != sabotaje_y)) {
			sem_wait(&tripu->semaforoTripulante);// VER SI PUEDE PASAR QUE EL PLANIFICADOR HAGA POST ANTES DE LLEGAR ACA
			sem_post(&sincroTripuPlanif);
			int xOrigen=tripu -> pos_x;
			int yOrigen=tripu -> pos_y;
			sem_wait(&operacionesTripulantes); //TODO CREO QUE ESTE SEMAFORO Y SU POST NO SON NECESARIOS, NADIE ADEMAS DEL TRIPU CAMBIA SU POSICION
			if (tripu -> pos_x > sabotaje_x) {
				tripu -> pos_x = tripu -> pos_x - 1;
			} else if (tripu -> pos_x < sabotaje_x) {
				tripu -> pos_x = tripu -> pos_x + 1;
			} else if (tripu -> pos_y > sabotaje_y) {
				tripu -> pos_y = tripu -> pos_y - 1;
			} else if (tripu -> pos_y < sabotaje_y) {
				tripu -> pos_y = tripu -> pos_y + 1;
			}
			sem_post(&operacionesTripulantes);
			sem_wait(&operacionesTripulantes);
			tripu->pasosAdar--;
			sem_post(&operacionesTripulantes);

			t_paquete* paqueteActualizaUbicacion = crear_paquete_con_codigo_op(ACTUALIZAR_UBICACION);

		//sem_wait(&operacionesTripulantes);
		agregar_entero_a_paquete(paqueteActualizaUbicacion, tripu->id);
		agregar_entero_a_paquete(paqueteActualizaUbicacion, tripu->id_patota);
		agregar_entero_a_paquete(paqueteActualizaUbicacion, tripu->pos_x);// TODO no agrego semaforo pq creo que nadie cambia la pos del tripu
		agregar_entero_a_paquete(paqueteActualizaUbicacion, tripu->pos_y);
		//sem_post(&operacionesTripulantes);
		
		int conexionActualizaUb = intentar_enviar_paquete(paqueteActualizaUbicacion, config_valores.ip_ram, config_valores.puerto_ram);
		// LIBERAR CONEXION

				//liberar_conexion(conexionActualizaUb);
		//BORRAR PAQUETE
				eliminar_paquete(paqueteActualizaUbicacion);
				sem_wait(&operacionesTripulantes);
						t_paquete* paqueteActualizaUbicacionMongo = crear_paquete_con_codigo_op(13);
						sem_post(&operacionesTripulantes);
						sem_wait(&operacionesTripulantes);
						int id=tripu->id;
						int x=tripu->pos_x;
						int y=tripu->pos_y;
						sem_post(&operacionesTripulantes);
						sem_wait(&operacionesTripulantes);
						agregar_entero_a_paquete(paqueteActualizaUbicacionMongo, id);
						agregar_entero_a_paquete(paqueteActualizaUbicacionMongo, x);// TODO no agrego semaforo pq creo que nadie cambia la pos del tripu
						agregar_entero_a_paquete(paqueteActualizaUbicacionMongo, y);
						agregar_entero_a_paquete(paqueteActualizaUbicacionMongo, xOrigen);
						agregar_entero_a_paquete(paqueteActualizaUbicacionMongo, yOrigen);
						sem_post(&operacionesTripulantes);
						sem_wait(&operacionesTripulantes);
						int conexionActualizaUbMongo;

						conexionActualizaUbMongo= crear_conexion(config_valores.ip_mongo, config_valores.puerto_mongo);
						enviar_paquete(paqueteActualizaUbicacionMongo, conexionActualizaUbMongo);
					//
						sem_post(&operacionesTripulantes);
						// LIBERAR CONEXION
						int cod_mongo= recibir_operacion(conexionActualizaUbMongo);
						liberar_conexion(conexionActualizaUbMongo);

				//BORRAR PAQUETE
				eliminar_paquete(paqueteActualizaUbicacionMongo);


		}
		t_paquete* paqueteMongo = crear_paquete_con_codigo_op(FSCK);
			//AGREGAR A PAQUETE
			agregar_entero_a_paquete(paqueteMongo,tripu->id);
			int conexionMongo = intentar_enviar_paquete(paqueteMongo, config_valores.ip_mongo, config_valores.puerto_mongo);

			eliminar_paquete(paqueteMongo);
			liberar_conexion(conexionMongo);
			sem_wait(&tripu->semaforoTripulante);

}

void realizarSabotaje2(tripulante* tripu){
	sem_wait(&operacionesTripulantes);
	int posXAntesDelSabotaje = tripu -> pos_x;
	int posYAntesDelSabotaje = tripu -> pos_y;
	tripu->pasosAdar= pasosAdar(tripu->pos_x,tripu->pos_y,sabotaje_x,sabotaje_y);
	sem_post(&operacionesTripulantes);
		while((tripu -> pos_x != sabotaje_x) || (tripu -> pos_y != sabotaje_y)) {
			sem_wait(&tripu->semaforoTripulante);// VER SI PUEDE PASAR QUE EL PLANIFICADOR HAGA POST ANTES DE LLEGAR ACA
			sem_post(&sincroTripuPlanif);
			int xOrigen=tripu -> pos_x;
			int yOrigen=tripu -> pos_y;
			sem_wait(&operacionesTripulantes); //TODO CREO QUE ESTE SEMAFORO Y SU POST NO SON NECESARIOS, NADIE ADEMAS DEL TRIPU CAMBIA SU POSICION
			if (tripu -> pos_x > sabotaje_x) {
				tripu -> pos_x = tripu -> pos_x - 1;
			} else if (tripu -> pos_x < sabotaje_x) {
				tripu -> pos_x = tripu -> pos_x + 1;
			} else if (tripu -> pos_y > sabotaje_y) {
				tripu -> pos_y = tripu -> pos_y - 1;
			} else if (tripu -> pos_y < sabotaje_y) {
				tripu -> pos_y = tripu -> pos_y + 1;
			}
			sem_post(&operacionesTripulantes);
			sem_wait(&operacionesTripulantes);
			tripu->pasosAdar--;
			sem_post(&operacionesTripulantes);

			t_paquete* paqueteActualizaUbicacion = crear_paquete_con_codigo_op(ACTUALIZAR_UBICACION);

		//sem_wait(&operacionesTripulantes);
		agregar_entero_a_paquete(paqueteActualizaUbicacion, tripu->id);
		agregar_entero_a_paquete(paqueteActualizaUbicacion, tripu->id_patota);
		agregar_entero_a_paquete(paqueteActualizaUbicacion, tripu->pos_x);// TODO no agrego semaforo pq creo que nadie cambia la pos del tripu
		agregar_entero_a_paquete(paqueteActualizaUbicacion, tripu->pos_y);
		//sem_post(&operacionesTripulantes);

		int conexionActualizaUb = intentar_enviar_paquete(paqueteActualizaUbicacion, config_valores.ip_ram, config_valores.puerto_ram);
		//int codop = recibir_operacion(conexionActualizaUb);
		// LIBERAR CONEXION

				//liberar_conexion(conexionActualizaUb);
		//BORRAR PAQUETE
				eliminar_paquete(paqueteActualizaUbicacion);
				sem_wait(&operacionesTripulantes);
						t_paquete* paqueteActualizaUbicacionMongo = crear_paquete_con_codigo_op(13);
						sem_post(&operacionesTripulantes);
						sem_wait(&operacionesTripulantes);
						int id=tripu->id;
						int x=tripu->pos_x;
						int y=tripu->pos_y;
						sem_post(&operacionesTripulantes);
						sem_wait(&operacionesTripulantes);
						agregar_entero_a_paquete(paqueteActualizaUbicacionMongo, id);
						agregar_entero_a_paquete(paqueteActualizaUbicacionMongo, x);// TODO no agrego semaforo pq creo que nadie cambia la pos del tripu
						agregar_entero_a_paquete(paqueteActualizaUbicacionMongo, y);
						agregar_entero_a_paquete(paqueteActualizaUbicacionMongo, xOrigen);
						agregar_entero_a_paquete(paqueteActualizaUbicacionMongo, yOrigen);
						sem_post(&operacionesTripulantes);
						sem_wait(&operacionesTripulantes);
						int conexionActualizaUbMongo;

						conexionActualizaUbMongo= crear_conexion(config_valores.ip_mongo, config_valores.puerto_mongo);
						enviar_paquete(paqueteActualizaUbicacionMongo, conexionActualizaUbMongo);
					//
						sem_post(&operacionesTripulantes);
						// LIBERAR CONEXION
						int cod_mongo= recibir_operacion(conexionActualizaUbMongo);
						liberar_conexion(conexionActualizaUbMongo);

				//BORRAR PAQUETE
				eliminar_paquete(paqueteActualizaUbicacionMongo);


		}
		t_paquete* paqueteMongo = crear_paquete_con_codigo_op(FSCK);
			//AGREGAR A PAQUETE
			agregar_entero_a_paquete(paqueteMongo,tripu->id);
			int conexionMongo = intentar_enviar_paquete(paqueteMongo, config_valores.ip_mongo, config_valores.puerto_mongo);

			eliminar_paquete(paqueteMongo);
			liberar_conexion(conexionMongo);
			sem_wait(&tripu->semaforoTripulante);
			sem_wait(&operacionesTripulantes);
			tripu->pasosAdar= pasosAdar(tripu->pos_x,tripu->pos_y,posXAntesDelSabotaje,posYAntesDelSabotaje);
			sem_post(&operacionesTripulantes);
			while((tripu -> pos_x != posXAntesDelSabotaje) || (tripu -> pos_y != posYAntesDelSabotaje)) {
			sem_wait(&tripu->semaforoTripulante);// VER SI PUEDE PASAR QUE EL PLANIFICADOR HAGA POST ANTES DE LLEGAR ACA
			if(tripu->elegidoPorElDestino==1){
				avisarleAmongo(tripu->id);
				realizarSabotaje((void*) tripu);
				sem_wait(&operacionesTripulantes);
					tripu->pasosAdar= pasosAdar(tripu->pos_x,tripu->pos_y,posXAntesDelSabotaje,posYAntesDelSabotaje);
				sem_post(&operacionesTripulantes);
				sem_wait(&tripu->semaforoTripulante);
			}
			sem_post(&sincroTripuPlanif2);
			int xOrigen=tripu -> pos_x;
			int yOrigen=tripu -> pos_y;
			sem_wait(&operacionesTripulantes); //TODO CREO QUE ESTE SEMAFORO Y SU POST NO SON NECESARIOS, NADIE ADEMAS DEL TRIPU CAMBIA SU POSICION
			if (tripu -> pos_x > posXAntesDelSabotaje) {
				tripu -> pos_x = tripu -> pos_x - 1;
			} else if (tripu -> pos_x < posXAntesDelSabotaje) {
				tripu -> pos_x = tripu -> pos_x + 1;
			} else if (tripu -> pos_y > posYAntesDelSabotaje) {
				tripu -> pos_y = tripu -> pos_y - 1;
			} else if (tripu -> pos_y < posYAntesDelSabotaje) {
				tripu -> pos_y = tripu -> pos_y + 1;
			}
			sem_post(&operacionesTripulantes);
			sem_wait(&operacionesTripulantes);
			tripu->pasosAdar--;
			sem_post(&operacionesTripulantes);

			t_paquete* paqueteActualizaUbicacion = crear_paquete_con_codigo_op(ACTUALIZAR_UBICACION);

		//sem_wait(&operacionesTripulantes);
		agregar_entero_a_paquete(paqueteActualizaUbicacion, tripu->id);
		agregar_entero_a_paquete(paqueteActualizaUbicacion, tripu->id_patota);
		agregar_entero_a_paquete(paqueteActualizaUbicacion, tripu->pos_x);// TODO no agrego semaforo pq creo que nadie cambia la pos del tripu
		agregar_entero_a_paquete(paqueteActualizaUbicacion, tripu->pos_y);
		//sem_post(&operacionesTripulantes);
		
		int conexionActualizaUb = intentar_enviar_paquete(paqueteActualizaUbicacion, config_valores.ip_ram, config_valores.puerto_ram);
		// LIBERAR CONEXION

				//liberar_conexion(conexionActualizaUb);
		//BORRAR PAQUETE
				eliminar_paquete(paqueteActualizaUbicacion);
				sem_wait(&operacionesTripulantes);
						t_paquete* paqueteActualizaUbicacionMongo = crear_paquete_con_codigo_op(13);
						sem_post(&operacionesTripulantes);
						sem_wait(&operacionesTripulantes);
						int id=tripu->id;
						int x=tripu->pos_x;
						int y=tripu->pos_y;
						sem_post(&operacionesTripulantes);
						sem_wait(&operacionesTripulantes);
						agregar_entero_a_paquete(paqueteActualizaUbicacionMongo, id);
						agregar_entero_a_paquete(paqueteActualizaUbicacionMongo, x);// TODO no agrego semaforo pq creo que nadie cambia la pos del tripu
						agregar_entero_a_paquete(paqueteActualizaUbicacionMongo, y);
						agregar_entero_a_paquete(paqueteActualizaUbicacionMongo, xOrigen);
						agregar_entero_a_paquete(paqueteActualizaUbicacionMongo, yOrigen);
						sem_post(&operacionesTripulantes);
						sem_wait(&operacionesTripulantes);
						int conexionActualizaUbMongo;

						conexionActualizaUbMongo= crear_conexion(config_valores.ip_mongo, config_valores.puerto_mongo);
						enviar_paquete(paqueteActualizaUbicacionMongo, conexionActualizaUbMongo);
					//
						sem_post(&operacionesTripulantes);
						// LIBERAR CONEXION
						int cod_mongo= recibir_operacion(conexionActualizaUbMongo);
						liberar_conexion(conexionActualizaUbMongo);

				//BORRAR PAQUETE
				eliminar_paquete(paqueteActualizaUbicacionMongo);
				
				sem_wait(&planificacionPausada); //ESTO ESTA POR SI VIENE UN SABOTAJE MIENTRAS ME MUEVO
				sem_post(&planificacionPausada); // PREGUNTAR SI TA BN O NO ES NECESARIO
		/*if(strcmp(config_valores.algoritmo,"RR")==0){
			int ciclosTotales= tripu->ciclos;
			if(ciclosTotales == config_valores.quantum){
				sem_wait(&operacionesTripulantes);
				tripu->ciclos=0;
				sem_post(&operacionesTripulantes);
				sem_wait(&tripu->semaforoTripulante);
			}
		}*/

		}

			return;
}

void generarOxigeno(int cantAEditar,int idTripu,int id_patota){
	t_paquete* paqueteMongo = crear_paquete_con_codigo_op(GENERAR_OXIGENO);
	//AGREGAR A PAQUETE
	agregar_entero_a_paquete(paqueteMongo, cantAEditar);
	agregar_entero_a_paquete(paqueteMongo, idTripu);
	sem_wait(&operacionesTripulantes);
	int conexionMongo = intentar_enviar_paquete(paqueteMongo, config_valores.ip_mongo, config_valores.puerto_mongo);
	//int cod_mongo= recibir_operacion(conexionMongo);
	sem_post(&operacionesTripulantes);
	int cod_mongo1= recibir_operacion(conexionMongo);
	eliminar_paquete(paqueteMongo);
	liberar_conexion(conexionMongo);
}
void consumirOxigeno(int cantAEditar,int idTripu,int id_patota){
	t_paquete* paqueteMongo = crear_paquete_con_codigo_op(CONSUMIR_OXIGENO);
	//AGREGAR A PAQUETE
	agregar_entero_a_paquete(paqueteMongo, cantAEditar);
	agregar_entero_a_paquete(paqueteMongo, idTripu);
	int conexionMongo = intentar_enviar_paquete(paqueteMongo, config_valores.ip_mongo, config_valores.puerto_mongo);
	int cod_mongo= recibir_operacion(conexionMongo);
	eliminar_paquete(paqueteMongo);
	liberar_conexion(conexionMongo);
}
void generarComida(int cantAEditar,int idTripu,int id_patota){
	t_paquete* paqueteMongo = crear_paquete_con_codigo_op(GENERAR_COMIDA);
	//AGREGAR A PAQUETE
	agregar_entero_a_paquete(paqueteMongo, cantAEditar);
	agregar_entero_a_paquete(paqueteMongo, idTripu);
	int conexionMongo = intentar_enviar_paquete(paqueteMongo, config_valores.ip_mongo, config_valores.puerto_mongo);
	int cod_mongo= recibir_operacion(conexionMongo);
	eliminar_paquete(paqueteMongo);
	liberar_conexion(conexionMongo);
}
void consumirComida(int cantAEditar,int idTripu,int id_patota){
	t_paquete* paqueteMongo = crear_paquete_con_codigo_op(CONSUMIR_COMIDA);
	//AGREGAR A PAQUETE
	agregar_entero_a_paquete(paqueteMongo, cantAEditar);
	agregar_entero_a_paquete(paqueteMongo, idTripu);
	int conexionMongo = intentar_enviar_paquete(paqueteMongo, config_valores.ip_mongo, config_valores.puerto_mongo);
	int cod_mongo= recibir_operacion(conexionMongo);
	eliminar_paquete(paqueteMongo);
	liberar_conexion(conexionMongo);
}
void generarBasura(int cantAEditar,int idTripu,int id_patota){
	t_paquete* paqueteMongo = crear_paquete_con_codigo_op(GENERAR_BASURA);
	//AGREGAR A PAQUETE
	agregar_entero_a_paquete(paqueteMongo, cantAEditar);
	agregar_entero_a_paquete(paqueteMongo, idTripu);
	int conexionMongo = intentar_enviar_paquete(paqueteMongo, config_valores.ip_mongo, config_valores.puerto_mongo);
	int cod_mongo= recibir_operacion(conexionMongo);
	eliminar_paquete(paqueteMongo);
	liberar_conexion(conexionMongo);
}
void descartarBasura(int cantAEditar,int idTripu,int id_patota){
	t_paquete* paqueteMongo = crear_paquete_con_codigo_op(DESCARTAR_BASURA);
	//AGREGAR A PAQUETE
	agregar_entero_a_paquete(paqueteMongo, cantAEditar);
	agregar_entero_a_paquete(paqueteMongo, idTripu);
	int conexionMongo = intentar_enviar_paquete(paqueteMongo, config_valores.ip_mongo, config_valores.puerto_mongo);
	int cod_mongo= recibir_operacion(conexionMongo);
	eliminar_paquete(paqueteMongo);
	liberar_conexion(conexionMongo);
}

void inicioTarea(int id,int idPatota){
	t_paquete* paqueteRam = crear_paquete_con_codigo_op(INICIO_TAREA);
	//AGREGAR A PAQUETE
	agregar_entero_a_paquete(paqueteRam, idPatota); // AGREGAMOS ID PATOTA
	agregar_entero_a_paquete(paqueteRam, id ); // AGREGAMOS ID TRIPU
	int conexionRam = intentar_enviar_paquete(paqueteRam, config_valores.ip_ram, config_valores.puerto_ram);

	eliminar_paquete(paqueteRam);
	liberar_conexion(conexionRam);

}

void finTarea(int id,int idPatota){
	t_paquete* paqueteRam = crear_paquete_con_codigo_op(FIN_TAREA);
	//AGREGAR A PAQUETE
	agregar_entero_a_paquete(paqueteRam, idPatota); // AGREGAMOS ID PATOTA
	agregar_entero_a_paquete(paqueteRam, id ); // AGREGAMOS ID TRIPU
	int conexionRam = intentar_enviar_paquete(paqueteRam, config_valores.ip_ram, config_valores.puerto_ram);

	eliminar_paquete(paqueteRam);
	liberar_conexion(conexionRam);

}

void inicioMongo(int idTripu ){
	t_paquete* paqueteMongo = crear_paquete_con_codigo_op(INICIO_TAREARANDOM);
	//AGREGAR A PAQUETE
	agregar_entero_a_paquete(paqueteMongo, idTripu);
	int conexionMongo = intentar_enviar_paquete(paqueteMongo, config_valores.ip_mongo, config_valores.puerto_mongo);
	int cod_mongo= recibir_operacion(conexionMongo);
	eliminar_paquete(paqueteMongo);
	liberar_conexion(conexionMongo);
}

void finOxigeno(int id){
	t_paquete* paqueteMongo = crear_paquete_con_codigo_op(FIN_OXIGENO);
		//AGREGAR A PAQUETE
		agregar_entero_a_paquete(paqueteMongo, id);
		int conexionMongo = intentar_enviar_paquete(paqueteMongo, config_valores.ip_mongo, config_valores.puerto_mongo);
		int cod_mongo= recibir_operacion(conexionMongo);
		eliminar_paquete(paqueteMongo);
		liberar_conexion(conexionMongo);
}

void finConsumirOxigeno(int id){
	t_paquete* paqueteMongo = crear_paquete_con_codigo_op(FIN_CONSUMIR_OXIGENO);
		//AGREGAR A PAQUETE
		agregar_entero_a_paquete(paqueteMongo, id);
		int conexionMongo = intentar_enviar_paquete(paqueteMongo, config_valores.ip_mongo, config_valores.puerto_mongo);
		int cod_mongo= recibir_operacion(conexionMongo);
		eliminar_paquete(paqueteMongo);
		liberar_conexion(conexionMongo);
}

void finComida(int id){
	t_paquete* paqueteMongo = crear_paquete_con_codigo_op(FIN_COMIDA);
		//AGREGAR A PAQUETE
		agregar_entero_a_paquete(paqueteMongo, id);
		int conexionMongo = intentar_enviar_paquete(paqueteMongo, config_valores.ip_mongo, config_valores.puerto_mongo);
		int cod_mongo= recibir_operacion(conexionMongo);
		eliminar_paquete(paqueteMongo);
		liberar_conexion(conexionMongo);
}

void finConsumircomida(int id){
	t_paquete* paqueteMongo = crear_paquete_con_codigo_op(FIN_CONSUMIR_COMIDA);
		//AGREGAR A PAQUETE
		agregar_entero_a_paquete(paqueteMongo, id);
		int conexionMongo = intentar_enviar_paquete(paqueteMongo, config_valores.ip_mongo, config_valores.puerto_mongo);
		int cod_mongo= recibir_operacion(conexionMongo);
		eliminar_paquete(paqueteMongo);
		liberar_conexion(conexionMongo);
}

void finBasura(int id){
	t_paquete* paqueteMongo = crear_paquete_con_codigo_op(FIN_BASURA);
		//AGREGAR A PAQUETE
		agregar_entero_a_paquete(paqueteMongo, id);
		int conexionMongo = intentar_enviar_paquete(paqueteMongo, config_valores.ip_mongo, config_valores.puerto_mongo);

		int cod_mongo= recibir_operacion(conexionMongo);
		eliminar_paquete(paqueteMongo);
		liberar_conexion(conexionMongo);
}

void finConsumirBasura(int id){
	t_paquete* paqueteMongo = crear_paquete_con_codigo_op(FIN_CONSUMIR_BASURA);
		//AGREGAR A PAQUETE
		agregar_entero_a_paquete(paqueteMongo, id);
		int conexionMongo = intentar_enviar_paquete(paqueteMongo, config_valores.ip_mongo, config_valores.puerto_mongo);
		int cod_mongo= recibir_operacion(conexionMongo);
		eliminar_paquete(paqueteMongo);
		liberar_conexion(conexionMongo);
}

void finMongo(int id){
	t_paquete* paqueteMongo = crear_paquete_con_codigo_op(FIN_TAREARANDOM);
		//AGREGAR A PAQUETE
		agregar_entero_a_paquete(paqueteMongo, id);
		int conexionMongo = intentar_enviar_paquete(paqueteMongo, config_valores.ip_mongo, config_valores.puerto_mongo);
		int cod_mongo= recibir_operacion(conexionMongo);
		eliminar_paquete(paqueteMongo);
		liberar_conexion(conexionMongo);
}




