/*
 * servidor.c
 *
 *  Created on: 3 mar. 2019
 *      Author: utnso
 */
#include "imongo.h"


void manejar_conexion_con_pasamano(int socket)
{

	pthread_t hilo_ubicacion;
		char* tarea;
		int cod_op = recibir_operacion(socket);
		switch(cod_op)
		{
		case GENERAR_OXIGENO:
			tarea= "Empezo generar oxigeno";
			generarOxigeno2(socket,tarea);
			log_info(logger, "Oxigeno generado");
			break;

		case CONSUMIR_OXIGENO:
			tarea= "Empezo consumir oxigeno";
			consumirOxigeno(socket,tarea);
			log_info(logger, "Oxigeno Consumido");

			break;
		case GENERAR_COMIDA:
			tarea= "Empezo generar comida";
			generarComida(socket,tarea);
			log_info(logger, "Comida generada");
			break;
		case CONSUMIR_COMIDA:
			tarea= "Empezo consumir comida";
			consumirComida(socket,tarea);
			log_info(logger, "Comida consumida");
			break;
		case GENERAR_BASURA:
			tarea= "Empezo generar basura";
			generarBasura2(socket,tarea);
			log_info(logger, "Basura generada");
			break;
		case DESCARTAR_BASURA:
			tarea= "Empezo descartar basura";
			consumirBasura2(socket,tarea);
			log_info(logger, "Basura descartada");
			break;
		case INICIAR_TRIPULANTE:
			iniciarTripualnte(socket);
			log_info(logger, "Tripulante iniciado");
			break;

		case ACTUALIZAR_UBICACION:
			//pthread_create(&hilo_ubicacion, NULL, (void*) obtenerParametros_ubicacion, socket);
			//pthread_detach(hilo_ubicacion);
			obtenerParametros_ubicacion(socket);
			log_info(logger, " Ubicacion tripulante actualizada");
			break;
		case INICIO_TAREARANDOM:
					tarea= "Empezo tarea no definida";
					obtenerParametros_tareas(socket,tarea);
					break;
		case FIN_CONSUMIR_BASURA:
			tarea= "Termino consumir basura";
			obtenerParametros_tareas(socket,tarea);
					break;
		case FIN_BASURA:
			tarea= "Termino agregar basura";
						obtenerParametros_tareas(socket,tarea);
					break;
		case FIN_CONSUMIR_COMIDA:
			tarea= "Termino consumir comida";
						obtenerParametros_tareas(socket,tarea);
					break;
		case FIN_COMIDA:
			tarea= "Termino agregar comida";
						obtenerParametros_tareas(socket,tarea);
					break;
		case FIN_CONSUMIR_OXIGENO:
			tarea= "Termino consumir oxigeno";
						obtenerParametros_tareas(socket,tarea);
					break;
		case FIN_OXIGENO:
			tarea= "Termino agregar oxigeno";
						obtenerParametros_tareas(socket,tarea);
					break;
		case FIN_TAREARANDOM:
			tarea= "Termino tarea no definida";
									obtenerParametros_tareas(socket,tarea);
									break;

		case CORRE_SABOTAJE:
			tarea="CORRIO AL SABOTAJE";
				obtenerParametros_tareas(socket,tarea);
				break;
		case FSCK:
			tarea="se resuelve el sabotaje";
			log_info(logger,"ANTES DE LA LLAMADA A LA FUNC");
				resolverSabotaje(socket,tarea);
				break;
		case OBTENER_BITACORA:
			manejar_obtener_bitacora(socket);
			break;
		case MENSAJE:
			obtenerParametros_ubicacion(socket);
			break;
		case PAQUETE:
			obtenerParametros_ubicacion(socket);
			break;
		case CERRAR:
			//x=0;
			break;
		case -1:
			log_error(logger, "el cliente se desconecto. Terminando servidor");
			exit(EXIT_FAILURE);
			break;
		default:
			printf("%d \n",cod_op);
			log_warning(logger, "Operacion desconocida. No quieras meter la pata");
			break;
		}
}


void manejar_clientes(int server_fd) //Thread para esperar clientes
{
	while(1)
	{

		int socket = esperar_cliente(server_fd);

		//Threads para recepcion / envio de info a clientes
		pthread_t t;
		//pthread_mutex_lock(&recib_clientes1);
		pthread_create(&t, NULL, (void*) manejar_conexion_con_pasamano, (void*) socket);
		pthread_detach(t);
		//pthread_mutex_unlock(&recib_clientes1);
		printf("Thread: %lu\n", t);
	}
}

void signalHandler(int sig){
	log_info(logger,"Me SABOTEAAAANN!!!");
	t_paquete* paquete_discord = crear_paquete_con_codigo_op(SABOTAJE);
	char** split = string_split(config_valores.posiciones_sabotaje[numeroSabotaje], "|");
	int posX= atoi(split[0]);
	int posY= atoi(split[1]);
	numeroSabotaje++;
	agregar_entero_a_paquete(paquete_discord, posX);
	agregar_entero_a_paquete(paquete_discord, posY); //COMO EL ENUNCIADO NO LO DICE, ME INVENTO LA POSICION
	int conexion_discord = intentar_enviar_paquete(paquete_discord, config_valores.ip_discord, config_valores.puerto_discord);
	// BORRAR PAQUETE
	 eliminar_paquete(paquete_discord);
	 // LIBERAR CONEXION
	 liberar_conexion(conexion_discord);
}

int main(void)
{
	/*void iterator(char* value)
	{
		printf("%s\n", value);
	}*/
	programaTerminado=0;
	numeroSabotaje=0;
	logger = iniciar_logger();
	cargar_configuracion();
	iniciar_file_system();
	int server_fd = iniciar_servidor(config_valores.ip_propia, config_valores.puerto_propio);
	pthread_t manejo_recepcion;
	pthread_create(&manejo_recepcion, NULL, (void*) manejar_clientes, (void*)server_fd);
	pthread_detach(manejo_recepcion);

	log_info(logger, "Servidor listo para recibir al cliente");
	/*obtenerBitacora(0 );
	obtenerBitacora(1 );
	obtenerBitacora(2 );*/
	/*sleep(1);
	agregarCaracter(  'O',   35);
	agregarCaracter(  'O',   2);
	agregarCaracter(  'B',   58);
	agregarCaracter(  'C',   58);
	agregarCaracter(  'C',   58);
	crearBitacora(1);
	crearBitacora(2);
	borrarCaracter('B',15);
	actualizarTarea(1,"hola idolo intergalactico");
	actualizarTarea(1,"hola idolo 1AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");*/
	/*crearBitacora(1);
		crearBitacora(2);
		crearBitacora(3);
			crearBitacora(4);


	/*solucionarSabotajes();

	solucionarSabotajes();

	solucionarSabotajes();

	solucionarSabotajes();

		solucionarSabotajes();

		solucionarSabotajes();

		solucionarSabotajes();

			solucionarSabotajes();

			solucionarSabotajes();*/

	signal(SIGUSR1,&signalHandler); //TODO VER SI VA ADENTRO DE UN WHILE pasarle la posicion correcta


	while (1)
			{
				sleep(1);
				char* leido = readline(">");
				if (string_equals_ignore_case(leido, "exit"))
				{
					free(copiaBlock);
					free(leido);
					//programaTerminado=1;

					break;
				}
				free(leido);

			}
	config_destroy(config_programa);
	return EXIT_SUCCESS;
}

void cargar_configuracion(void)
{
	config_programa = config_create("../imongo.config"); //Leo el archivo de configuracion

	if (config_programa == NULL) {
		perror("Archivo de configuracion de imongo no encontrado");
		return;
	}

	//config_valores.ruta_logger =	 	config_get_string_value(config, "ARCHIVO_LOG");
	config_valores.ip_propia = 			config_get_string_value(config_programa, "IP_PROPIA");
	config_valores.puerto_propio = 		config_get_string_value(config_programa, "PUERTO_PROPIO");
	config_valores.ip_discord = 		config_get_string_value(config_programa, "IP_DISCORD");
	config_valores.puerto_discord = 	config_get_string_value(config_programa, "PUERTO_DISCORD");
	config_valores.ip_ram = 		    config_get_string_value(config_programa, "IP_RAM");
	config_valores.puerto_ram =     	config_get_string_value(config_programa, "PUERTO_RAM");

	config_valores.punto_montaje =			config_get_string_value(config_programa, 	"PUNTO_MONTAJE");
	config_valores.tiempo_sincronizacion = 	config_get_int_value(config_programa, 	"TIEMPO_SINCRONIZACION");
	config_valores.posiciones_sabotaje = 	config_get_array_value(config_programa, "POSICIONES_SABOTAJE"); //TODO aca va array de string value
	config_valores.block_size =  (uint32_t) config_get_int_value(config_programa, "BLOCK_SIZE");
	config_valores.blocks = 	 (uint32_t) config_get_int_value(config_programa, "BLOCKS");
	config_valores.superBloque_creado = 	config_get_string_value(config_programa, "SUPERBLOQUE_CREADO");
	config_valores.bitmap = 	            config_get_int_value(config_programa, "BITMAP"); //NO SE SI REALMENTE TIENE QUE IR

	return;
}

t_log* iniciar_logger(void)
{
	t_log * logger;
	if((logger = log_create("imongo.log","IMONGO",1,LOG_LEVEL_INFO)) == NULL){//el segundo item va el nombre que quiera, el 1 representa que la consola esta activa, si no iria 0
		printf("no pude crear el logger");
		exit (1);
	}
	return logger;
}  //el log level es el que me pide el tp y esta el struct en las commons


void generarOxigeno(struct_tarea* tarea_struct){//int socket,char* tarea){
	int size;
	char * buffer;
	int desp = 0;
	buffer = recibir_buffer(&size, tarea_struct->socket);
	int cantidad = leer_entero(buffer, &desp);
	int id=leer_entero(buffer, &desp);
	free(buffer);
	pthread_mutex_lock(&sincro_oxigeno); //TODO VER SI ESTA BIEN O SI LA SECCION CRITICA ES MUY GRANDE
	agregarCaracter('O', cantidad);
	pthread_mutex_unlock(&sincro_oxigeno);

	actualizarTarea(id,tarea_struct->tarea);
	enviar_ok(tarea_struct->socket);

	return;
}

void generarOxigeno2(int socket, char* tarea){//int socket,char* tarea){
	int size;
	char * buffer;
	int desp = 0;
	buffer = recibir_buffer(&size,  socket);
	int cantidad = leer_entero(buffer, &desp);
	int id=leer_entero(buffer, &desp);
	free(buffer);
	pthread_mutex_lock(&sincro_oxigeno); //TODO VER SI ESTA BIEN O SI LA SECCION CRITICA ES MUY GRANDE
	agregarCaracter('O', cantidad);
	pthread_mutex_unlock(&sincro_oxigeno);

	actualizarTarea(id, tarea);
	enviar_ok( socket);

	return;
}

void consumirOxigeno(int socket,char* tarea){
	int size;
	char * buffer;
	int desp = 0;
	buffer = recibir_buffer(&size, socket);
	int cantidad = leer_entero(buffer, &desp);
	int id=leer_entero(buffer, &desp);
	free(buffer);

	pthread_mutex_lock(&sincro_oxigeno);
	borrarCaracter('O', cantidad);
	pthread_mutex_unlock(&sincro_oxigeno);

	actualizarTarea(id,tarea);
	enviar_ok(socket);

	return;
}

void generarComida(int socket,char* tarea){
	int size;
	char * buffer;
	int desp = 0;
	buffer = recibir_buffer(&size, socket);
	int cantidad = leer_entero(buffer, &desp);
	int id=leer_entero(buffer, &desp);
	free(buffer);

	pthread_mutex_lock(&sincro_comida);
	agregarCaracter('C', cantidad);
	pthread_mutex_unlock(&sincro_comida);

	actualizarTarea(id,tarea);
	enviar_ok(socket);

	return;
}

void consumirComida(int socket,char* tarea){
	int size;
	char * buffer;
	int desp = 0;
	buffer = recibir_buffer(&size, socket);
	int cantidad = leer_entero(buffer, &desp);
	int id=leer_entero(buffer, &desp);
	free(buffer);

	pthread_mutex_lock(&sincro_comida);
	borrarCaracter('C', cantidad);
	pthread_mutex_unlock(&sincro_comida);

	actualizarTarea(id,tarea);
	enviar_ok(socket);

	return;
}

void generarBasura(struct_tarea* tarea_struct){//int socket,char* tarea){
	int size;
	char * buffer;
	int desp = 0;
	buffer = recibir_buffer(&size, tarea_struct->socket);
	int cantidad = leer_entero(buffer, &desp);
	int id=leer_entero(buffer, &desp);
	free(buffer);

	pthread_mutex_lock(&sincro_basura);
	agregarCaracter('B', cantidad);
	pthread_mutex_unlock(&sincro_basura);

	actualizarTarea(id,tarea_struct->tarea);
	enviar_ok(tarea_struct->socket);

	return;
}

void generarBasura2(int socket, char* tarea){//int socket,char* tarea){
	int size;
	char * buffer;
	int desp = 0;
	buffer = recibir_buffer(&size,  socket);
	int cantidad = leer_entero(buffer, &desp);
	int id=leer_entero(buffer, &desp);
	free(buffer);

	pthread_mutex_lock(&sincro_basura);
	agregarCaracter('B', cantidad);
	pthread_mutex_unlock(&sincro_basura);

	actualizarTarea(id,  tarea);
	enviar_ok( socket);

	return;
}

void consumirBasura(struct_tarea* tarea_struct){//int socket,char* tarea){
	int size;
	char * buffer;
	int desp = 0;
	buffer = recibir_buffer(&size, tarea_struct->socket);
	int cantidad = leer_entero(buffer, &desp);
	int id=leer_entero(buffer, &desp);
	free(buffer);

	pthread_mutex_lock(&sincro_basura);
	borrarCaracter('B', cantidad);
	pthread_mutex_unlock(&sincro_basura);

	actualizarTarea(id,tarea_struct->tarea);
	enviar_ok(tarea_struct->socket);

	return;
}

void consumirBasura2(int socket,char* tarea){
	int size;
	char * buffer;
	int desp = 0;
	buffer = recibir_buffer(&size,  socket);
	int cantidad = leer_entero(buffer, &desp);
	int id=leer_entero(buffer, &desp);
	free(buffer);

	pthread_mutex_lock(&sincro_basura);
	borrarCaracter('B', cantidad);
	pthread_mutex_unlock(&sincro_basura);

	actualizarTarea(id, tarea);
	enviar_ok( socket);

	return;
}

void iniciarTripualnte(int socket){
	int size;
		char * buffer;
		int desp = 0;
		buffer = recibir_buffer(&size, socket);
		int id = leer_entero(buffer, &desp);
		free(buffer);
		crearBitacora(id);
		enviar_ok2(socket);

}

void obtenerParametros_ubicacion(int socket){
			char * buffer;
			int size;
			buffer = recibir_buffer(&size, socket);
			int desp = 0;

			int id = leer_entero(buffer, &desp);
			int x_nueva = leer_entero(buffer, &desp);
			int y_nueva = leer_entero(buffer, &desp);
			int x_vieja = leer_entero(buffer, &desp);
			int y_vieja = leer_entero(buffer, &desp);

			struct_ubicacion* ubicacion= malloc(sizeof(struct_ubicacion));
			ubicacion->id=id;
			ubicacion->x_nueva=x_nueva;
			ubicacion->y_nueva=y_nueva;
			ubicacion->x_vieja=x_vieja;
			ubicacion->y_vieja=y_vieja;
			//pthread_t hilo_ubicacion;
			pthread_mutex_lock(&actualiz_tarea);
			//pthread_create(&hilo_ubicacion, NULL, (void*) actualizarUbicacionBitacora, (void*) ubicacion);
			//pthread_detach(hilo_ubicacion);
			actualizarUbicacionBitacora((void*)ubicacion);
			pthread_mutex_unlock(&actualiz_tarea);
			free(buffer);
			free(ubicacion);
			enviar_ok(socket);



}

void obtenerParametros_tareas(int socket,char* tarea){
	int size;
	 char * buffer;
	 int desp = 0;
	 buffer = recibir_buffer(&size, socket);
	 int id = leer_entero(buffer, &desp);
	 free(buffer);
	 pthread_mutex_lock(&actualiz_tarea);
	 actualizarTarea(id,tarea);
	 pthread_mutex_unlock(&actualiz_tarea);
	 enviar_ok(socket);
}

void resolverSabotaje(int socket,char*tarea){
	int size;
	 char * buffer;
	 int desp = 0;
	 buffer = recibir_buffer(&size, socket);
	 int id = leer_entero(buffer, &desp);
	 free(buffer);
	 log_info(logger,"dentro de la func");
	 actualizarTarea(id,tarea);
	 solucionarSabotajes();
}

void manejar_obtener_bitacora(int socket){
	int size;
	char * buffer;
	int desp = 0;
	buffer = recibir_buffer(&size, socket);
	int id = leer_entero(buffer, &desp);
	obtenerBitacora(id );
	liberar_conexion(socket );

}


// FALTA OBTENER BITACORA
void obtenerBitacora(int id ){
	char* idTripu= string_itoa(id);
	char* bitacora_tripu = string_new();
	string_append(&bitacora_tripu, bitacora_path);
	string_append(&bitacora_tripu, "Tripulante");
	string_append(&bitacora_tripu, idTripu);
	string_append(&bitacora_tripu, ".ims");
	t_config* bitacora_config = config_create(bitacora_tripu);
	int cantidadDeCaracteres =  config_get_int_value(bitacora_config, "SIZE");
	char** bloquesUsados = config_get_array_value(bitacora_config, "BLOCKS");
	char* bitacora= malloc(cantidadDeCaracteres+1);
	int escritas=0;
	int bloquesLen=-1;
	if(cantidadDeCaracteres!=0){
	 while (bloquesUsados[++bloquesLen] != NULL){}

	for(int i=0;i<bloquesLen-1;i++){
		int bloque= atoi(bloquesUsados[i]);
		for(int j=0;j< config_valores.block_size;j++){

			memcpy(bitacora+j+(i*config_valores.block_size),copiaBlock+j+(bloque*config_valores.block_size),sizeof(char));
			escritas++;
		}
	}
	cantidadDeCaracteres= cantidadDeCaracteres-escritas;
	int bloque= atoi(bloquesUsados[bloquesLen-1]);
	for(int i=0;i<cantidadDeCaracteres;i++){
		memcpy(bitacora+i+((bloquesLen-1)*config_valores.block_size),copiaBlock+i+(bloque *config_valores.block_size),sizeof(char));
	}
	}else{
		bitacora= "bitacora vacia";
	}
	//printf("%s \n",bitacora);
	//CREAR CONEXION
		int socket_cliente;
		socket_cliente= crear_conexion(config_valores.ip_discord,config_valores.puerto_discord);
		enviar_mensaje_bitacora(bitacora,socket_cliente);
		// LIBERAR CONEXION
		liberar_conexion(socket_cliente );

}

void enviar_ok(int socket){
	pthread_mutex_lock(&respuesta_mongo);
	t_paquete* tripus = crear_paquete_con_codigo_op(OK);

	enviar_paquete(tripus,socket);
	eliminar_paquete(tripus);
	pthread_mutex_unlock(&respuesta_mongo);
}

void enviar_ok2(int socket){
	pthread_mutex_lock(&respuesta_mongo);
	t_paquete* tripus = crear_paquete_con_codigo_op(OK2);

	enviar_paquete(tripus,socket);
	eliminar_paquete(tripus);
	pthread_mutex_unlock(&respuesta_mongo);
}



