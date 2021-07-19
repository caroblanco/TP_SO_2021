/*
 * conexiones.h
 *
 *  Created on: 2 mar. 2019
 *      Author: utnso
 */

#ifndef UTILS_H_
#define UTILS_H_


#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<signal.h>
#include<unistd.h>
#include<netdb.h>
#include<commons/log.h>
#include<commons/collections/list.h>
#include<commons/string.h>
#include<string.h>
#include<readline/readline.h>
#include"pthread.h"
#include <dirent.h>
#include <errno.h>
#include <arpa/inet.h>


typedef enum
{
	MENSAJE,
	PAQUETE,
	INICIAR_PATOTA,
	ENVIAR_OP,
	LISTAR_TRIPULANTES,
	GENERAR_OXIGENO,
	CONSUMIR_OXIGENO,
	GENERAR_COMIDA,
	CONSUMIR_COMIDA,
	GENERAR_BASURA,
	DESCARTAR_BASURA,
	EXPULSAR_TRIPULANTE,
	INICIAR_TRIPULANTE,
	ACTUALIZAR_UBICACION,
	TERMINO_IO,
	INICIO_TAREA,
	FIN_TAREA,
	MOVERSE,
	INICIO_TAREARANDOM,
	FIN_CONSUMIR_BASURA,
	FIN_BASURA,
	FIN_CONSUMIR_COMIDA,
	FIN_COMIDA,
	FIN_CONSUMIR_OXIGENO,
	FIN_OXIGENO,
	FIN_TAREARANDOM,
	SABOTAJE,
	FSCK,
	CORRE_SABOTAJE,
	OBTENER_BITACORA,
	CAMBIAR_ESTADO,
	OK,
	FAIL,
	CERRAR,
	OK2
	
}op_code;

pthread_mutex_t recib_clientes11;

typedef enum{
	PAGINACION,
	SEGMENTACION
}esquema;
typedef struct
{
	int size;
	void* stream;
} t_buffer;

typedef struct
{
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;

typedef struct {
	
	uint32_t idPatota; 				//Identificador de la patota
	uint32_t tareas; 			//Direccion logica del inicio de las tareas
	
}t_pcb;

typedef struct {
	
	uint32_t idTripulante; 				//ID del tripulante
	uint32_t posX; 				//Pos en eje x
	uint32_t posY; 				//Pos en eje y
	uint32_t proxInstruccion; 	//Id de la proxima instruccion
	uint32_t puntero_pcb; 		//Direccion logica del PCB del tripulante, su patota
	char estado; 				//Estado del tripulante (N/R/E/B)
	
}t_tcb;

pthread_mutex_t mutexOk;

int existe_archivo(char* );
int string_arr_size(char**);

// GESTIONAR CONEXIONES //
int crear_conexion(char* ip, char* puerto);
void liberar_conexion(int socket_cliente);

void enviar_mensaje(char* mensaje, int socket_cliente);

// CREAR PAQUETES //
t_paquete* crear_paquete(void);
t_paquete* crear_super_paquete(void);
t_paquete* crear_paquete_con_codigo_op(op_code );

// AGREGAR A PAQUETE //
void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio);
void agregar_entero_a_paquete(t_paquete*, int);
void agregar_array_string_a_paquete(t_paquete* paquete, char** arr);

// GESTIONAR PAQUETE //
void enviar_paquete(t_paquete* paquete, int socket_cliente);
int intentar_enviar_paquete(t_paquete * paquete, char* ip, char* puerto);
void eliminar_paquete(t_paquete* paquete);
uint32_t conectar_socket_a(char* , uint32_t );
// INICIAR SERVIDOR //
void* recibir_buffer(int*, int);

int iniciar_servidor(char*, char*);
int esperar_cliente(int);
t_list* recibir_paquete(int);
void recibir_mensaje(int);
int recibir_operacion(int);

// LEER PAQUETES //
int   leer_entero(char*, int*);
char** leer_array_string(char*, int*);
int* leer_array_entero(char*, int*);
char* leer_string(char*, int*);

int existe_dir(char* path);

//Pasar de int a char
char intToChar(int unEntero);

//SEPARAR TAREAS
char** normalizarTareas(char*);

int stringArraySize(char**);
void recibir_mensaje_bitacora(int);
void enviar_mensaje_bitacora(char*  , int  );
t_paquete* crear_paquete_con_codigo_op2(op_code  );
void enviar_codop(op_code  , int  );

void enviarFail(int socket);
void enviarOK(int socket);

void liberarStringArray(char** stringArray);

int tamanioStringArray(char** a);

#endif /* UTILS_H_ */





/////////////////////////////////////////////////////////OTRO TP//////////////////////////////////////////////////////////////////////////////

/*
#define PENDIENTE 0
#define CONFIRMADO 1
#define LISTO 2
#define ENTREGADO 3
typedef enum
{
	MENSAJE,			// mensaje simple (string)
	PAQUETE,			// paquete de strings
	COMANDO,			// header de paquete con distintos formatos
	OK,					// retorno de operacion
	FAIL,				// retorno de operacion / especial: comando/ argumentos invalidos en consola
	EXIT,				// especial: salir de la consola
	HELP,				// especial: obtener comandos validos con sus argumentos
	HANDSHAKE_APP,
	HANDSHAKE_COMANDA,
	HANDSHAKE_SINDICATO,
	HANDSHAKE_RESTAURANTE,
	CONSULTAR_RESTAURANTES,
	RTA_LISTA_RESTAURANTES,
	RESTAURANTE,		// obtener restaurante 
	RESTAURANTE_SELEC,	// seleccionar restaurante
	INFO_PLATOS,		// consultar platos
	RTA_INFO_PLATOS,	// retorno consultar platos
	ID_PEDIDO,			// crear pedido
	INFO_PEDIDO,		// consultar pedido
	PEDIDO_RECIBIDO,	// utilizado por consultar estado de cliente 
	CREAR_PEDIDO,		//
	RTA_CREAR_PEDIDO,	//
	GUARDAR_PEDIDO,		//
	ANADIR_PLATO,		//
	GUARDAR_PLATO,		// 
	CONSULTAR_PEDIDO,
	OBTENER_PEDIDO,		//
	RTA_OBTENER_PEDIDO,	//
	PLATO_LISTO,		//
	FINALIZAR_PEDIDO,	//
	TERMINAR_PEDIDO,
	CONFIRMAR_PEDIDO,	//
	RECETA,				// obtener receta
}op_code;

//t_log* logger;			// logger que vamos a utilizar globalmente


typedef enum //para argumentos de super_crear_paquete
{
    INT,
    STRING,
    ARR_INT,
    ARR_STR
}data_type; 

//Funciones utiles para comunicacion

int iniciar_servidor(char*, char*);
int esperar_cliente(int);
void manejar_conexion(int);

void super_agregar_a_paquete(t_paquete*, int, ...);
void agregar_array_entero_a_paquete(t_paquete* paquete, int* arr);
void agregar_paquete_a_paquete(t_paquete*, t_paquete*);
int intentar_enviar_paquete(t_paquete * paquete, char* ip, char* puerto);
void enviar_codop(op_code, int);

void* recibir_buffer(int*, int);
char* leer_string(char*, int*);
int   leer_entero(char*, int*); 
char** leer_array_string(char*, int*);
int* leer_array_entero(char*, int*);

t_list* recibir_paquete(int);
void recibir_mensaje(int);
int recibir_operacion(int);
//Retorno obtener restaurante
t_restaurante* recibir_restaurante(int);
void mostrar_restaurante(t_restaurante*);
void mostrar_receta(t_receta*);
//Retorno consultar restaurantes
void recibir_lista_restaurantes(int);
//Retorno obtener pedido
t_list* recibir_pedido(int);
void devolver_pedido(t_list*, int);

void enviar_platos(char**, int);
char** recibir_platos(int);
int recibir_id_pedido(int);
void enviar_id_pedido(int, int);
void enviar_id_pedido_creado(int, int);

int min(int,int);
int max(int, int);
//Utilidades de listas
void print_str(char*);
void list_iterate_and_add(t_list*, t_paquete*);
char* iToStr(int);

int string_arr_size(char**);
int* string_array_to_int_array(char**);
int int_arr_size(int* a);

int existe_archivo(char*);
int existe_dir(char* path);
t_list* archivos_en_dir(char*);
void _leer_consola_haciendo(void(*accion)(char*));

void lock(pthread_mutex_t);
void unlock(pthread_mutex_t);

#endif  UTILS_H_ 



void* recibir_buffer(int* size, int socket_cliente)
{
	void * buffer;
	//printf("recibiendo\n");
	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	//printf("tamanio %d\n",*size);
	buffer = malloc(*size);
	
	if(buffer != NULL){
		//printf("recibiendo buffer\n");
		recv(socket_cliente, buffer, *size, MSG_WAITALL);
	}else
		printf("MALLOC ERROR OF SIZE %d\n\n", *size);

	//printf("buffer location %d\n", (int) buffer);
	return buffer;
}


void agregar_entero_a_paquete(t_paquete* paquete, int x) // Agrega un entero a un paquete (ya creado)
{
	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(int));
	
	memcpy(paquete->buffer->stream + paquete->buffer->size, &x, sizeof(int));

	paquete->buffer->size += sizeof(int);
	
}

void agregar_array_string_a_paquete(t_paquete* paquete, char** arr)
{
	int size = string_arr_size(arr);
	agregar_entero_a_paquete(paquete,size);
	for(int i = 0; i < size; i++)
		agregar_a_paquete(paquete, arr[i], strlen(arr[i])+1);
} */
