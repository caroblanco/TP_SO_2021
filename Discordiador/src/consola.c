#include "consola.h"
#include <readline/readline.h>
#include <readline/history.h>

int idUltimaPatotaCreada = 0;
int idUltimoTripulanteCreado = 0;

void manejarConsola(void){
	log_info(logger,"INICIANDO CONSOLA");
	int planificacionIniciada=0;
	int firstTime=0;
	while (1) {
	  printf("ingrese un comando: \n");
	  char* leido = readline(">");

	  char** split = string_split(leido, " ");
      int len = string_arr_size(split);

	  if (string_equals_ignore_case(split[0], "help"))
	          {
	              printf("Comandos: \n"
	  				"  INICIAR_PATOTA \n"
	  				"  LISTAR_TRIPULANTES \n"
	  				"  EXPULSAR_TRIPULANTE \n"
	  				"  INICIAR_PLANIFICACION \n"
	  				"  PAUSAR_PLANIFICACION \n"
	  				"  OBTENER_BITACORA \n"
	            	"  exit \n"
	  				"\n");
	              free(split);
	              free(leido);
	          }
		//INICIAR_PATOTA 5 /home/utnso/tareas/tareasPatota5.txt 1|1 3|4
	   else if(string_equals_ignore_case(split[0], "INICIAR_PATOTA") || string_equals_ignore_case(split[0], "IP"))
	          {
		   	   	  /* if(planificacionIniciada==0){
		   	   		   printf("No se inicio la planificacion, la patota no puede ser creada \n");
					   free(split);
		   	   		   free(leido);
		   	   	   } else {*/
		   	   		int cantNuevosTripulantes= atoi(split[1]);
					char* tareas= leerArchivo(split[2]);
					char** posiciones = crearPosiciones(split, len, cantNuevosTripulantes); //ASIGNAMOS LAS POSICIONES

			//		valgrind --show-leak-kinds=all --leak-check=full -v --log-file="(nombreDelArchivo.txt)" ./(ejecutable)
						//CREAR CONEXION
							int socket_cliente;
							socket_cliente= crear_conexion(config_valores.ip_ram,config_valores.puerto_ram);
						//CREAR PAQUETE
							t_paquete* nuevoPaquete = crear_paquete_con_codigo_op(INICIAR_PATOTA);
						//AGREGAR A PAQUETE
							//agregar_entero_a_paquete(nuevoPaquete, cantNuevosTripulantes); 
							agregar_a_paquete(nuevoPaquete,tareas,(strlen(tareas) + 1));
							agregar_entero_a_paquete(nuevoPaquete, idUltimaPatotaCreada); 
							agregar_entero_a_paquete(nuevoPaquete, cantNuevosTripulantes); //PARA QUE SE FIJEN SI ENTRA EN MEM

							//TODO VER SI AGREGAR LA CANTIDAD DE LA PATOTA PARA QUE MI RAM SE FIJE SI ENTRA


						//ENVIAR PAQUETE
							enviar_paquete(nuevoPaquete, socket_cliente);
						//INICIAR PATOTA
							log_info(logger,"Se envio el paquete iniciar patota a mi-ram");
							int codop = recibir_operacion(socket_cliente);

								if(codop == OK){
							iniciarPatota(cantNuevosTripulantes, idUltimaPatotaCreada, idUltimoTripulanteCreado,posiciones);
							idUltimaPatotaCreada++;
							idUltimoTripulanteCreado += cantNuevosTripulantes;
								}else if(codop==FAIL){
									printf("no habia suficiente espacio en memoria \n");
									log_info(logger,"no habia suficiente espacio en memoria \n");
								}else{
									printf("algo fallo mi rey \n");
								}


		   	   			free(split);
		   	   			free(leido);
						// BORRAR PAQUETE
						eliminar_paquete(nuevoPaquete);
						// LIBERAR CONEXION
						liberar_conexion(socket_cliente);
		   	   	   //}

	          }
	   else if(string_equals_ignore_case(split[0], "LISTAR_TRIPULANTES") || string_equals_ignore_case(split[0], "lt"))
	   	    	{
					int canTripulantesNuevos = list_size(tripulantesNuevos);
					int cantTripulantesReady = list_size(tripulantesReady);
					int canTripulanteBlockIO = list_size(tripulantesBlockIO);
					int canTripulanteBlock = list_size(tripulantesBlock);
					int cantTripulantesExec = list_size(tripulantesExecute);
					int cantTripulantesFinaliz = list_size(tripulantesFinalizados);
					int cantTripulantesBloquEmerg = list_size(bloqueadosEmergencia);

					/*printf("------------------------------------------------------------------------------------\n
					Estado de la Nave: " + fechaYHora + "\n");*/
					for(int i=0; i< canTripulantesNuevos; i++)
					{
						imprimirTripulante(list_get(tripulantesNuevos,i));
					}
					for(int i=0; i< cantTripulantesReady; i++)
					{
						imprimirTripulante(list_get(tripulantesReady,i));
					}
					for(int i=0; i< cantTripulantesExec; i++)
					{
						imprimirTripulante(list_get(tripulantesExecute,i));
					}
					for(int i=0; i< canTripulanteBlock; i++)
					{
						imprimirTripulante(list_get(tripulantesBlock,i));
					}
					for(int i=0; i< canTripulanteBlockIO; i++)
										{
											imprimirTripulante(list_get(tripulantesBlockIO,i));
										}
					for(int i=0; i< cantTripulantesFinaliz; i++)
										{
											imprimirTripulante(list_get(tripulantesFinalizados,i));
										}
					for(int i=0; i< cantTripulantesBloquEmerg; i++)
										{
											imprimirTripulante(list_get(bloqueadosEmergencia,i));
										}
					printf("------------------------------------------------------------------------------------\n");
					free(split);
		   	   		free(leido);
	   	    	}
	   else if(string_equals_ignore_case(split[0], "EXPULSAR_TRIPULANTE") || string_equals_ignore_case(split[0], "e"))
	   	   	          {
					if(firstTime!=1){
		   	   		   printf("No se inicio la planificacion\n");
					   free(split);
		   	   		   free(leido);
		   	   	   } else {
								int idTripulanteAELiminar = atoi(split[1]); 
								tripulante* unTripu;
							//ELIMINAR TRIPULANTE
								int cantTripulantesNuevos = list_size(tripulantesNuevos);
								int cantTripulantesReady = list_size(tripulantesReady);
								int cantTripulantesExec = list_size(tripulantesExecute);
								int cantTripulantesBlock = list_size(tripulantesBlock);
								int canTripulanteBlockIO = list_size(tripulantesBlockIO);
								int cantTripulantesFinaliz = list_size(tripulantesFinalizados);
								int cantTripulantesBloquEmerg = list_size(bloqueadosEmergencia);

								for(int i=0; i< cantTripulantesNuevos; i++) {
									 unTripu = list_get(tripulantesNuevos,i);
										if(unTripu->id == idTripulanteAELiminar) {
												list_remove(tripulantesNuevos, i);
												cantTripulantesNuevos--;
												i--;
												log_info(logger,"el tripulante fue removido de la lista de nuevos");
										}
								}
								for(int i=0; i< cantTripulantesReady; i++) {
										unTripu = list_get(tripulantesReady,i);
										if(unTripu->id == idTripulanteAELiminar) {
												list_remove(tripulantesReady, i);
												cantTripulantesReady--;
												i--;
												log_info(logger,"el tripulante fue removido de la lista de ready");

										}
								}
								for(int i=0; i< cantTripulantesExec; i++) {
										unTripu = list_get(tripulantesExecute,i);
										if(unTripu->id == idTripulanteAELiminar) {
												list_remove(tripulantesExecute, i);
												i--;
												cantTripulantesExec--;
												log_info(logger,"el tripulante fue removido de la lista de exec");

										}
								}
								for(int i=0; i< cantTripulantesBlock; i++) {
										unTripu = list_get(tripulantesBlock,i);
										if(unTripu->id == idTripulanteAELiminar) {
												list_remove(tripulantesBlock, i);
												i--;
												cantTripulantesBlock--;
												log_info(logger,"el tripulante fue removido de la lista de block");

										}
								}
								for(int i=0; i< canTripulanteBlockIO; i++) {
											unTripu = list_get(tripulantesBlockIO,i);
											if(unTripu->id == idTripulanteAELiminar) {
												list_remove(tripulantesBlockIO, i);
												canTripulanteBlockIO--;
												i--;
												log_info(logger,"el tripulante fue removido de la lista de blockIO");

											}
									}
								for(int i=0; i< cantTripulantesFinaliz; i++) {
											unTripu = list_get(tripulantesFinalizados,i);
											if(unTripu->id == idTripulanteAELiminar) {
												list_remove(tripulantesFinalizados, i);
												cantTripulantesFinaliz--;
												i--;

												log_info(logger,"el tripulante fue removido de la lista de finaliz");

											}
									}
								for(int i=0; i< cantTripulantesBloquEmerg; i++) {
											unTripu = list_get(bloqueadosEmergencia,i);
											if(unTripu->id == idTripulanteAELiminar) {
												list_remove(bloqueadosEmergencia, i);
												cantTripulantesBloquEmerg--;
												i--;
												log_info(logger,"el tripulante fue removido de la lista de bloquEmerg");

											}
									}
							//CREAR CONEXION
								int socket_cliente;
								socket_cliente= crear_conexion(config_valores.ip_ram,config_valores.puerto_ram);
							//CREAR PAQUETE
								t_paquete* nuevoPaquete = crear_paquete_con_codigo_op(EXPULSAR_TRIPULANTE);
							//AGREGAR A PAQUETE
								agregar_entero_a_paquete(nuevoPaquete, idTripulanteAELiminar);
							//ENVIAR PAQUETE
								enviar_paquete(nuevoPaquete, socket_cliente);
								log_info(logger,"Se envio el paquete expulsar tripulante a mi-ram");
		   	   	   	   		free(split);
		   		            free(leido);
							// BORRAR PAQUETE
							eliminar_paquete(nuevoPaquete);
							// LIBERAR CONEXION
							liberar_conexion(socket_cliente);

	   	   	          }
					}
	   else if(string_equals_ignore_case(split[0], "INICIAR_PLANIFICACION")|| string_equals_ignore_case(split[0], "PL"))
	   	   	          {
		   planificacionIniciada=1;
		   if(firstTime==0){
			    firstTime=1;
			    if(strcmp(config_valores.algoritmo,"FIFO")==0){
				pthread_t planificacion_corto;
				pthread_create(&planificacion_corto, NULL, (void*) planificacionFIFO, NULL);
				pthread_detach(planificacion_corto);
				sem_post(&planificacionPausada);

			    }else if(strcmp(config_valores.algoritmo,"RR")==0){
			    	pthread_t planificacion_corto;
			    	pthread_create(&planificacion_corto, NULL, (void*) planificacionRR, NULL);
			    	pthread_detach(planificacion_corto);
			    	sem_post(&planificacionPausada);
			    	}
				}else{
			    	sem_post(&planificacionPausada);
			    }

		   

           free(split);
           free(leido);

	   	   	          }
	   else if(string_equals_ignore_case(split[0], "PAUSAR_PLANIFICACION") || string_equals_ignore_case(split[0], "pp"))
	   	   	          {
		   	   	   	  planificacionIniciada=0;
		   	   	   	  sem_wait(&planificacionPausada);
		              free(split);
		              free(leido);

	   	   	          }
	   else if(string_equals_ignore_case(split[0], "OBTENER_BITACORA") || string_equals_ignore_case(split[0], "ob"))
	   	   	          {
						int id=  atoi(split[1]);
						//CREAR CONEXION
							int socket_cliente;
							socket_cliente= crear_conexion(config_valores.ip_mongo,config_valores.puerto_mongo);
						//CREAR PAQUETE
							t_paquete* nuevoPaquete = crear_paquete_con_codigo_op(OBTENER_BITACORA);
						//AGREGAR A PAQUETE
							agregar_entero_a_paquete(nuevoPaquete, id);
						//ENVIAR PAQUETE
							enviar_paquete(nuevoPaquete, socket_cliente);

							eliminar_paquete(nuevoPaquete);
							// LIBERAR CONEXION
							liberar_conexion(socket_cliente);
		   	   	   	   	free(split);
		   		        free(leido);

	   	   	          }
	   else if(string_equals_ignore_case(split[0], "EXIT") || string_equals_ignore_case(split[0], "ex"))
	   	   	   	          {
		   int cantTripulantesFinaliz = list_size(tripulantesFinalizados);
			for(int i=0; i< cantTripulantesFinaliz; i++) {
				tripulante* unTripu;
						unTripu = list_get(tripulantesFinalizados,i);
							list_remove(tripulantesFinalizados, i);
							cantTripulantesFinaliz--;
							i--;
							free(unTripu);
							log_info(logger,"el tripulante fue removido de la lista de finaliz");

				}
	   	   	   	free(split);
		        free(leido);
				prog_andando=0;
		        return;
	   	   }
   }
}

char** crearPosiciones(char** split, int len, int tripulantes) {
	char** posiciones= malloc((tripulantes+1)*sizeof(char*));
	for(int i=3;i<(tripulantes+3);i++) {
		if (i < len) {
			posiciones[i-3] = split[i];
		} else {
			posiciones[i-3] = "0|0";
		}
	}
	//posiciones[tripulantes+1]="\0";
	return posiciones; // VER SI ESTA BIEN LO QUE DEVUELVE
}
/*
char* leerArchivo(char* ruta){
	    char str1;
	    int i=0;
	    FILE *f = fopen(ruta, "r");
		str1 = fgetc(f);
		while (str1 != EOF)
			{
	            str1 = fgetc(f);
	            i++;
			}

		char* leido = malloc(i * sizeof(char));
		i=0;
		fseek(f, 0, SEEK_SET);
		str1 = fgetc(f);
		while (str1 != EOF)
			{
	            leido[i]=str1;
	            str1 = fgetc(f);
	            i++;
			}
		//leido[i]='\0'; // VER EL WARNING
		fclose(f);
	    return leido;
} */

char* leerArchivo(char* unPath){
	
	char tareas[100];
	
	strcpy(tareas,"../tareas/");
	
	strcat(tareas,unPath);
	
    FILE* file = fopen(tareas,"r");
    if(file == NULL)
    {
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long int size = ftell(file);
    rewind(file);

    char* content = calloc(size + 1, 1);

    fread(content,1,size,file);
    
    fclose(file);
    
    
    return content;


}

void imprimirTripulante(tripulante* unTripu) {
	char* stringEstado;
	
	if(unTripu->estado == 'N') {
		stringEstado = "NEW";
	}
	else if(unTripu->estado == 'R'){
		stringEstado = "READY";
	}
	else if(unTripu->estado == 'E'){
		stringEstado = "EXEC";
	}else if(unTripu->estado == 'F'){
		stringEstado = "FINALIZ";
	}else {
		stringEstado = "BLOCK";
	} 
	printf("Tripulante: %d                          Patota: %d                          Status: %s  \n", unTripu->id, unTripu->id_patota, stringEstado);
}








































