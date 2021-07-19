#ifndef INIT_FILE_SYSTEM_H_
#define INIT_FILE_SYSTEM_H_


#include <commons/collections/dictionary.h>
#include <string.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

#include<math.h>
#include<commons/log.h>
#include<commons/string.h>

#include "imongo.h"


t_log* logger;
 
//void* block;


typedef struct
{
    int size;
    int block_count;
    char** blocks;
    char* caracter_llenado;
    char* md5Archivo;

} OXIGENO_CONFIG;
OXIGENO_CONFIG config_oxigeno;

typedef struct
{
    int size;
    int block_count;
    char** blocks;
    char* caracter_llenado;
    char* md5Archivo;

} COMIDA_CONFIG;
COMIDA_CONFIG config_comida;

typedef struct
{
    int size;
    int block_count;
    char** blocks;
    char* caracter_llenado;
    char* md5Archivo;

} BASURA_CONFIG;
BASURA_CONFIG config_basura;



t_log* logger;


void cargar_configuracion();
void crear_paths();
t_log* iniciar_logger(void);
void crear_metadata();
void crear_superBloque();
void crear_blocks_path();
void crear_bitacora_path();
void crear_superBloque();
void crearYmanejar_blocks();
void cargar_superBLoque();
void cargar_configuracionBasura();
void cargar_configuracionOxigeno();
void cargar_configuracionComida();
void crear_matadataBaura();
void crear_matadataComida();
void crear_matadataOxigeno();
void verificar_dir(char* );
void crear_metadata_path();

#endif /* INIT_FILE_SYSTEM_H_ */












