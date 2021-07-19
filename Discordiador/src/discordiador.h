/*
 * Cliente.h
 *
 *  Created on: 28 feb. 2019
 *      Author: utnso
 */

#ifndef TP0_H_
#define TP0_H_

#include <utils.h>
#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include <semaphore.h>
#include <pthread.h>
#include <stdarg.h>
#include "tripulante.h"

#define SIN_ASIGNAR 0
#define READY 1
#define EXEC 2
#define BLOCK 3
#define BLOCK_IO 4

t_log* logger;
pthread_mutex_t variable_sabotaje;
int var_sabotaje;
int sabotaje_x;
int sabotaje_y;
int prog_andando;

typedef struct
{
    //char* ruta_logger;

    char* ip_propia;
    char* ip_ram;
    char* ip_mongo;
    char* puerto_mongo;
    char* puerto_ram;
    char* puerto_propio;
    int tiempo_ciclo_cpu;
    int multitarea;
    char* algoritmo;
    //int default_x;
    //int default_y;
    int dur_sabotaje;
    int quantum;

} APP_config;
APP_config config_valores;

typedef struct
{
    int socket;
    int socket_anterior;
} t_conexiones;

void cargar_configuracion();

t_log* iniciar_logger(void);
t_config* leer_config(void);
void leer_consola(t_log*);
void paquete(int);
void terminar_programa(int, t_log*, t_config*);
void finalizar_programa(int, t_log*);
void manejarSabotaje(int  );



#endif /* TP0_H_ */
