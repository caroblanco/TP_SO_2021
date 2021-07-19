
//LD_LIBRARY_PATH="/home/utnso/tp-2021-1c-Los-Operativos/shareds/Debug/" ./Mi-RAM-HQ para correr en consola

//o usar export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:~/tp-2021-1c-Los_Brogrammers/utiles/Debug/ antes de ejecutar

//valgrind --leak-check=full --show-leak-kinds=definite,indirect --track-origins=yes --log-file=valgrind-out.txt ./miRamHQ

//valgrind --tool=helgrind --log-file=helgrind-out.txt ./miRamHQ


#include "mi-ram.h"

int main(void){
	
	cargar_configuracion();
	
	//logger = iniciar_logger();
	logger = log_create("../miRam.log", "Mi-RAM", 0, LOG_LEVEL_DEBUG);
	
	signal(SIGUSR1, &signalCompactacion);
	signal(SIGUSR2, &signalDump);

	mem = iniciarMemoria();

	if(!mem){
		return 0;
	}
	
	

	crearMapaVacio();

	//thread para tripulantes

	int server_fd = iniciar_servidor(config_valores.ip, config_valores.puerto);

	pthread_t manejoTripus;
	pthread_create(&manejoTripus, NULL, (void*) manejar_tripus,(void*)server_fd);
	pthread_detach(manejoTripus);


	while (1)
			{
				sleep(1);

				char* leido = readline(">");
				char** split = string_split(leido, " ");
				if (string_equals_ignore_case(split[0], "exit"))
				{
					free(leido);
					liberarStringArray(split);
					break;
				}else if(string_equals_ignore_case(split[0],"showS"))
				{
					showbitarr(bitMapSegment);
				}else if(string_equals_ignore_case(split[0],"showMP"))
				{
					showbitarr(frames_ocupados_ppal);
				}else if(string_equals_ignore_case(split[0],"showMV"))
				{
					showbitarr(frames_ocupados_virtual);
				}else if(string_equals_ignore_case(split[0],"tripulantesSeg"))
				{	
					void mostrarTripulantes(t_tabla_tripulante* unaTabla)
					{
						log_info(logger,"En el segmento es %d esta: \n", unaTabla->seg_tcb->id);
						mostrarTripulante(unaTabla->idTripulante);
					}
					
					list_iterate(tablaDeSegmentosDeTripulantes, (void*)mostrarTripulantes);
					
				}else if(string_equals_ignore_case(split[0],"tripulantesPag"))
				{	
					void mostrarT(t_tabla_pagina* unaTabla)
					{
						mostrarListaTripulantesPag(unaTabla->idTripus);
					}
					
					list_iterate(tablaDePaginas, (void*) mostrarT);
					
				}else if(string_equals_ignore_case(split[0],"patotas"))
				{	
					void mostrarPatotas(t_tabla_patota* unaTabla)
						{
							printf("La pcb de la patota %d esta entre %d y  %d \n", unaTabla->idPatota, unaTabla->pcb->base,(unaTabla->pcb->base + unaTabla->pcb->limite) );
							printf("Las tareas de la patota %d esta entre %d y  %d \n", unaTabla->idPatota, unaTabla->tareas->base,(unaTabla->tareas->base + unaTabla->tareas->limite) );
							
							
						}
					//pthread_mutex_lock(&mutexListaDePatotas);
					list_iterate(tablaDeSegmentosDePatotas, (void*)mostrarPatotas);
					//pthread_mutex_unlock(&mutexListaDePatotas);
					
				}else if(string_equals_ignore_case(split[0],"dump"))
				{
					dump();
				}else if(string_equals_ignore_case(split[0],"compactar"))
				{
					compactacion();
				}else if(string_equals_ignore_case(split[0],"tablas"))
				{
					void mostrarPaginas(t_tabla_pagina* unaTabla)
					{
						list_iterate(unaTabla->paginas, (void*)mostrarPagina);
					}
					list_iterate(tablaDePaginas, (void*)mostrarPaginas);
				}
				free(leido);
				liberarStringArray(split);
			}

	liberarMemoria();
	//liberar_conexion(server_fd);
	//liberarConfig();
	return EXIT_SUCCESS;
	
}



void manejar_tripus(int server_fd){ //RECIBO TRIPUS
	
	while(1){

		//log_info(logger, "Servidor listo para recibir clientes");
		int socketCliente = esperar_cliente(server_fd);

		pthread_t t;
		pthread_create(&t, NULL, (void*) manejarConexion, (void*)socketCliente);
		pthread_detach(t);
	}
}

void manejarConexion(int socket_cliente){ 
	
	//log_info(logger, "Se conecto un cliente!!");
	int codigoOperacion = recibir_operacion(socket_cliente);

	switch(codigoOperacion){
		case MENSAJE:
			//recibirMensaje(socket_cliente);
			break;
		case INICIAR_PATOTA:
			iniciarPatota(socket_cliente);
		break;
		case INICIAR_TRIPULANTE:
			log_info(logger, "Sale tripulante con papas");
			iniciarTripulante(socket_cliente);
		break;
		case EXPULSAR_TRIPULANTE:
			expulsarTripulante(socket_cliente);
		break;
		case ENVIAR_OP:
			enviarOperacion(socket_cliente);
		break;
		case ACTUALIZAR_UBICACION:
			actualizarUbicacion(socket_cliente);
		break; 
		case CAMBIAR_ESTADO:
			cambiarEstado(socket_cliente);
			break;
		case -1:
			//log_info(logger, "El tripulante se desconecto.");
		break;
	}

}
void cargar_configuracion(void)
{
	config = config_create("../mi-ram.config"); //Leo el archivo de configuracion

	if (config == NULL) {
		perror("Archivo de configuracion de MI-RAM no encontrado");
		exit(EXIT_FAILURE);
	}

	config_valores.tamanio_memoria =	        config_get_int_value(config,    "TAMANIO_MEMORIA");
	config_valores.esquema_memoria = 	        config_get_string_value(config, "ESQUEMA_MEMORIA");
	config_valores.tamanio_pagina =             config_get_int_value(config,    "TAMANIO_PAGINA");
	config_valores.tamanio_swap = 	            config_get_int_value(config,    "TAMANIO_SWAP");
	config_valores.path_swap = 		            config_get_string_value(config, "PATH_SWAP");
	config_valores.algoritmo_reemplazo =     	config_get_string_value(config, "ALGORITMO_REEMPLAZO");
	config_valores.puerto =	                    config_get_string_value(config, "PUERTO");
	config_valores.ip = 						config_get_string_value(config, "IP");
	config_valores.criterio_seleccion = 		config_get_string_value(config, "CRITERIO_SELECCION");
	
	//config_destroy(config);
}



void signalCompactacion(int sig){
	log_info(logger, "Recibi la senial de compactar, compactando...");
	compactacion();
}
void signalDump(int sig){
	
	dump();
}


void dump(){
	
	if(string_equals_ignore_case(config_valores.esquema_memoria , "SEGMENTACION")){
		dumpSegmentacion();
	}else if(string_equals_ignore_case(config_valores.esquema_memoria , "PAGINACION")){
		dumpPaginacion();
	}	
}
