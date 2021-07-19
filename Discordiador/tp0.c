
/*
 * main.c
 *
 *  Created on: 28 feb. 2019
 *      Author: utnso
 */

#include "tp0.h"

int main(void)
{
	/*---------------------------------------------------PARTE 2-------------------------------------------------------------*/
	int conexion;
	char* ip;
	char* puerto;
	char* valor;
	char* stringConsola;

	t_log* logger;
	t_config* config;

	logger = iniciar_logger();

	log_info(logger,"soy un log");

	config = leer_config();

	ip= config_get_string_value(config,"IP");
	puerto= config_get_string_value(config,"PUERTO");
	valor= config_get_string_value(config,"CLAVE");

	log_info(logger,"lei la ip %s y el puerto %s",ip,puerto);

	log_info(logger,"la clave: %s",valor);
	leer_consola(logger);


	/*---------------------------------------------------PARTE 3-------------------------------------------------------------*/

	//antes de continuar, tenemos que asegurarnos que el servidor esté corriendo porque lo necesitaremos para lo que sigue.

	//crear conexion
	conexion= crear_conexion(ip,puerto);
	//enviar CLAVE al servirdor
	enviar_mensaje(valor,conexion);

	paquete(conexion);

	terminar_programa(conexion, logger, config);
}

t_log* iniciar_logger(void)
{
	t_log * logger;
	if((logger = log_create("tp0.log","TP0",1,LOG_LEVEL_INFO)) == NULL){//el segundo item va el nombre que quiera, el 1 representa que la consola esta activa, si no iria 0
		printf("no pude crear el logger");
		exit (1);
	}
	return logger;
} //el log level es el que me pide el tp y esta el struct en las commons

t_config* leer_config(void)
{
	t_config * config;
	if ((config=config_create("./tp0.config"))==NULL){
		printf("no se pudo crear la config");
		exit (2);
	}
	return config;
}

void leer_consola(t_log* logger)
{
	char* leido;
	while( strcmp(leido= readline(">"),"")!=0){
		log_info(logger,"lo leido por consola: %s",leido);
			free(leido);

	};


}

void paquete(int conexion)
{
	//Ahora toca lo divertido!
	char* leido;
	t_paquete* paquete;
	paquete= crear_paquete();
	while( strcmp(leido= readline(">"),"")!=0){

		agregar_a_paquete(paquete,leido,strlen(leido) + 1);
				free(leido);

		};
	enviar_paquete(paquete,conexion);
	eliminar_paquete(paquete);
	return;


}

void terminar_programa(int conexion, t_log* logger, t_config* config)
{
	if (logger != NULL){
		log_destroy(logger);
	}
	if (config != NULL){
		config_destroy(config);
	}
	liberar_conexion(conexion);
}