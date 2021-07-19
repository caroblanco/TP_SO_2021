

#include "discordiador.h"
#include "consola.h"

void manejar_conexion_con_pasamano(t_conexiones* conexiones)
{
	int socket = conexiones->socket;
	int socket_anterior = conexiones->socket_anterior;

		int cod_op = recibir_operacion(socket);


		switch(cod_op)
		{
		case MENSAJE:
			recibir_mensaje(socket);
			break;
		case PAQUETE:

			break;

		case SABOTAJE:
			log_info(logger,"ME SABOTEARON PA");
			manejarSabotaje(socket);
			break;
		case OBTENER_BITACORA:
			recibir_mensaje_bitacora(socket);
	     break;
		case -1:
			//log_error(logger, "el cliente se desconecto");
			printf("se cerro la conexion / error\n");
			//disconnected = 1;
			break;
		default:
			printf("\nOPCODE: %d\n",cod_op);
			//log_warning(logger, "Operacion desconocida. No quieras meter la pata");
			exit(EXIT_FAILURE);
			break;
		}
}

void manejar_clientes(int server_fd) //Thread para esperar clientes
{
	while(1)
	{
		t_conexiones conexiones;
		conexiones.socket =  esperar_cliente(server_fd);
		conexiones.socket_anterior = 0;
		//Threads para recepcion / envio de info a clientes
		pthread_t t;
		pthread_create(&t, NULL, (void*) manejar_conexion_con_pasamano, (void*) &conexiones);
		pthread_detach(t);
		//printf("Thread: %lu\n", t);
	}
}


int main(void)
{
	int conexion;
	var_sabotaje=0;
	sabotaje_x=0;
	sabotaje_y=0;
	tripulantesReady =      list_create();
	tripulantesNuevos=      list_create();
	tripulantesExecute=     list_create();
	tripulantesBlockIO=     list_create();
	tripulantesBlock=       list_create();
	tripulantesFinalizados= list_create();
	bloqueadosEmergencia=   list_create();
	puerto_inicial=5800;


// TODO IMPORTANTE: SI LA PLANIFICACION ES FIFO, PONER QUANTUM -1
	cargar_configuracion();
	sem_init(&planificacion,0,config_valores.multitarea);
	sem_init(&operacionesTripulantes,0,1);
	sem_init(&planificacionPausada,0,0);
	sem_init(&sincroTripuPlanif,0,0);
	sem_init(&sincroTripuPlanif2,0,0);
	sem_init(&sabotaje,0,1);
	prog_andando= 1;
	logger = iniciar_logger();

	log_info(logger,"soy un log");

	log_info(logger,"lei la ip %s y el puerto %s",config_valores.ip_ram,config_valores.puerto_ram);

	//crear conexion
	//conexion= crear_conexion(config_valores.ip_ram,config_valores.puerto_ram);
	//enviar CLAVE al servirdor
	//enviar_mensaje("DISCORDIADOR" ,conexion);

	//liberar_conexion(conexion);
	int server_fd = iniciar_servidor(config_valores.ip_propia,config_valores.puerto_propio);
	pthread_t manejo_recepcion;
	pthread_create(&manejo_recepcion, NULL, (void*) manejar_clientes, (void*)server_fd);
	pthread_detach(manejo_recepcion); //Va a estar corriendo siempre a la espera de una solicitud de apertura de socket
	pthread_t manejo_consola;
		pthread_create(&manejo_consola, NULL, (void*) manejarConsola, NULL);
		pthread_detach(manejo_consola);

	while (prog_andando)
	{
		sleep(1);
		/*char* leido = readline(">");
		if (string_equals_ignore_case(leido, "exit"))
		{
			free(leido);
			break;
		}
		free(leido);*/
	}

	//finalizar_programa(conexion, logger);
}

t_log* iniciar_logger(void)
{
	t_log * logger;
	if((logger = log_create("discordiador.log","DISCORDIADOR",1,LOG_LEVEL_INFO)) == NULL){//el segundo item va el nombre que quiera, el 1 representa que la consola esta activa, si no iria 0
		printf("no pude crear el logger");
		exit (1);
	}
	return logger;
} //el log level es el que me pide el tp y esta el struct en las commons


void cargar_configuracion(void)
{
	t_config* config = config_create("../discordiador.config"); //Leo el archivo de configuracion

	if (config == NULL) {
		perror("Archivo de configuracion de APP no encontrado");
		return;
	}

	//config_valores.ruta_logger =	 	config_get_string_value(config, "ARCHIVO_LOG");
	config_valores.ip_propia = 			config_get_string_value(config, "IP_PROPIA");
	config_valores.puerto_propio = 		config_get_string_value(config, "PUERTO_PROPIO");
	config_valores.ip_mongo = 		    config_get_string_value(config, "IP_MONGO");
	config_valores.puerto_mongo = 	    config_get_string_value(config, "PUERTO_IMONGO");
	config_valores.ip_ram = 		    config_get_string_value(config, "IP_RAM");
	config_valores.puerto_ram =     	config_get_string_value(config, "PUERTO_RAM");

	config_valores.tiempo_ciclo_cpu =	config_get_int_value(config, 	"RETARDO_CICLO_CPU");
	config_valores.multitarea = 		config_get_int_value(config, 	"GRADO_MULTITAREA");
	config_valores.algoritmo = 			config_get_string_value(config, "ALGORITMO");
	config_valores.dur_sabotaje = 		config_get_int_value(config, 	"DURACION_SABOTAJE");
	config_valores.quantum = 			config_get_int_value(config, 	"QUANTUM");

	//config_destroy(config);
}

void finalizar_programa(int conexion, t_log* logger )
{
	if (logger != NULL){
		log_destroy(logger);
	}

	liberar_conexion(conexion);
}

void manejarSabotaje(int socket){
	int size;
		 char * buffer;
		 int desp = 0;
		 buffer = recibir_buffer(&size, socket);
		 int pos_x = leer_entero(buffer, &desp);
		 int pos_y= leer_entero(buffer, &desp);

			sabotaje_x=pos_x;
			sabotaje_y=pos_y;
		 pthread_mutex_lock(&variable_sabotaje);
		 var_sabotaje=1;
		 pthread_mutex_unlock(&variable_sabotaje);
		 free(buffer);
		 //FALTA MANEJAR EL SABOTAJE
		 return;


}
