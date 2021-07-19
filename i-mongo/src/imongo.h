/*
 * servidor.h
 *
 *  Created on: 3 mar. 2019
 *      Author: utnso
 */

#ifndef IMONGO_H_
#define IMONGO_H_

#include <pthread.h>
#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include <semaphore.h>
#include <utils.h>
#include <stdarg.h>
#include<commons/bitarray.h>
#include <sys/mman.h>

#include <fcntl.h>

pthread_mutex_t sincro_block;
pthread_mutex_t sincro_bitmap;
pthread_mutex_t sincro_oxigeno;
pthread_mutex_t sincro_comida;
pthread_mutex_t sincro_basura;
pthread_mutex_t actualiz_tarea;
pthread_mutex_t sincro_ub;
pthread_mutex_t bitacora;
pthread_mutex_t respuesta_mongo;
pthread_mutex_t recib_clientes;
pthread_mutex_t recib_clientes1;

//int server_fd ;
int numeroSabotaje;
t_bitarray* bitmap;
int tamanoBitmap;
int puerto;
int programaTerminado;

t_config* config_programa;

char* metadata_path;
char* superBloque_file_path;
char* block_path;
char* bitacora_path;
void* copiaBlock; 

typedef struct
{
    //char* ruta_logger;

   int id;
   int x_nueva;
   int y_nueva;
   int x_vieja;
   int y_vieja;

} struct_ubicacion;

typedef struct
{
    //char* ruta_logger;

   int id;
   int socket;

} struct_ok;

typedef struct
{
    //char* ruta_logger;

   int socket;
   char* tarea;

} struct_tarea;

typedef struct
{
    //char* ruta_logger;

    char* ip_propia;
    char* ip_ram;
    char* ip_discord;
    char* puerto_discord;
    char* puerto_ram;
    char* puerto_propio;
    char* punto_montaje;
    int tiempo_sincronizacion;
    char** posiciones_sabotaje;
    uint32_t block_size;   //PREGUNTAR SI PUEDO USAR INT EN VEZ DE UNINT
    uint32_t blocks;
    char* superBloque_creado;
    int bitmap; //NO SE SI ES NECESARIO

} FILE_CONFIG;
FILE_CONFIG config_valores;

typedef struct {
	uint32_t block_size;
	uint32_t blocks;
	int bitmap;  //SIGO SIN SABER QUE CHINGADOS ES
} FS_superBloque;

FS_superBloque fs_superBloque;

char* metadata_file_path_oxigeno;
char* metadata_file_path_comida;
char* metadata_file_path_basura;

t_log* logger;


typedef struct
{
    int socket;
    int socket_anterior;
} t_conexiones;

t_log* logger;
void generarOxigeno2(int  , char*  );
void cargar_configuracion();
void recibir_patota(int );
t_log* iniciar_logger(void);
void agregarCaracter(char  , int  );
void borrarCaracter(char , int );
void iniciar_file_system();
void generarOxigeno(struct_tarea*);
void consumirOxigeno(int,char*);
void generarComida(int,char*);
void consumirComida(int,char*);
void generarBasura(struct_tarea*  );
void consumirBasura(struct_tarea*);
void iniciarTripualnte(int);
void crearBitacora(int );
void obtenerParametros_ubicacion(int  );
void obtenerParametros_tareas(int,char*);
void actualizarUbicacionBitacora(struct_ubicacion*); //int,int,int,int,int);
void actualizarTarea(int  ,char*  );
int buscarPrimerBloqueLibre();
void solucionarSabotajes();
int primerEspacioLibre(int , char );
void resolverSabotaje(int  ,char* );
void obtenerBitacora(int  );
void manejar_obtener_bitacora(int);
char* leerArchivo(char*  );
char* calcularMd5(int  , char  );
void enviar_ok(int );
void enviar_ok2(int );
void generarBasura2(int  , char*  );
void consumirBasura2(int  , char*);


#endif /* IMONGO_H_ */

