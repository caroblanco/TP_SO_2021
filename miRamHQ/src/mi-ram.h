#ifndef MIRAM_H_
#define MIRAM_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <commons/bitarray.h>
#include <readline/readline.h>
#include <utils.h>
#include <pthread.h>
#include <stdarg.h>
#include <math.h>
#include "mapa.h"
#include "memoria.h"
#include "opComunes.h"



typedef struct
{
	char* ruta_logger;
    int tamanio_memoria; 		// tamanio en bytes de la memoria real
    char* esquema_memoria; 		// Esquema de administración de memoria, puedeser: SEGMENTACION o PAGINACION
    int tamanio_pagina; 		// Tamaño en bytes de la cada página
    int tamanio_swap; 			// Tamaño en bytes del área de SWAP
    char* path_swap; 			// Ubicación del archivo de SWAP
    char* algoritmo_reemplazo; 	// LRU o CLOCK - Indica el algoritmo de reemplazo a utilizar en el esquema de paginación
    char* criterio_seleccion;   // El tipo de algoritmo de selección de partición libre a utilizar en el esquema de segmentación  (FF/BF)
	char* puerto;
	char* ip;
} Mi_RAM_config;

Mi_RAM_config config_valores;

void cargar_configuracion(void);
void manejar_tripus(int socket);
void manejarConexion(int socket);



//

void signalCompactacion(int);
void signalDump(int);
void dump();

int mem;
t_config* config;
t_log* logger;
pthread_mutex_t mutexSuper;

#endif /*MIRAM_H_*/
