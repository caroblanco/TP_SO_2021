/*
 * conexiones.c
 *
 *  Created on: 2 mar. 2019
 *      Author: utnso
 */

#include "utils.h"

int existe_archivo(char* path)
{
	int ret = 0;
	FILE* f = fopen(path,"r");
	if (f != NULL)
	{
		ret = 1;
		fclose(f);
	}
	return ret;
}

int existe_dir(char* path)
{
	DIR* dir = opendir(path);
	if (dir) {
		closedir(dir);
		return 1;
	} else if (ENOENT == errno) {
		return 0;
	} else {
		return -1;
	}
}

char intToChar(int unEntero){

    char a;

    a = unEntero + '0';

    return a;

}

int string_arr_size(char** a)
{
    int i = 0;
    while(a[i] != NULL)
    {
        i++;
    }
    return i;
}

void* serializar_paquete(t_paquete* paquete, int bytes)
{
	void * magic = malloc(bytes);
	int desplazamiento = 0;

	memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento+= paquete->buffer->size;

	return magic;
}

uint32_t conectar_socket_a(char* ip, uint32_t puerto){
    struct sockaddr_in direccionServidor;
    direccionServidor.sin_family = AF_INET;
    direccionServidor.sin_addr.s_addr = inet_addr(ip);
    direccionServidor.sin_port = htons(puerto);

    uint32_t cliente = socket(AF_INET, SOCK_STREAM,0);
    if (connect(cliente,(void*) &direccionServidor, sizeof(direccionServidor)) != 0){
        return -1;
    }
    return cliente;
}

int crear_conexion(char *ip, char* puerto)
{
	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(ip, puerto, &hints, &server_info);

	int socket_cliente = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);

	if(connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen) == -1)
	{
		printf("error conectandose al socket. reintentando.\n");
		socket_cliente = -1;
		//sleep(1);
	}
	freeaddrinfo(server_info);

	return socket_cliente;
}

void enviar_mensaje(char* mensaje, int socket_cliente)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = MENSAJE;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = strlen(mensaje) + 1;
	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size);

	int bytes = paquete->buffer->size + 2*sizeof(int);

	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
	eliminar_paquete(paquete);
}


void crear_buffer(t_paquete* paquete)
{
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = 0;
	paquete->buffer->stream = NULL;
}

t_paquete* crear_super_paquete(void)
{
	//me falta un malloc!
	t_paquete* paquete = malloc(sizeof(t_paquete));

	//descomentar despues de arreglar
	//paquete->codigo_operacion = PAQUETE;
	//crear_buffer(paquete);
	return paquete;
}

t_paquete* crear_paquete(void)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = PAQUETE;
	crear_buffer(paquete);
	return paquete;
}

t_paquete* crear_paquete_con_codigo_op(op_code codigo_op)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = codigo_op;
	crear_buffer(paquete);
	return paquete;
}

void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio)
{
	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio + sizeof(int));

	memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanio, sizeof(int));
	memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), valor, tamanio);

	paquete->buffer->size += tamanio + sizeof(int);
}

void enviar_mensaje_bitacora(char* mensaje, int socket_cliente)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = OBTENER_BITACORA;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = strlen(mensaje) + 1;
	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size);

	int bytes = paquete->buffer->size + 2*sizeof(int);

	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
	eliminar_paquete(paquete);
}

void enviar_paquete(t_paquete* paquete, int socket_cliente)
{
	int bytes = paquete->buffer->size + 2*sizeof(int);
	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
}

int intentar_enviar_paquete(t_paquete * paquete, char* ip, char* puerto)
{
	int conexion;
	//Intento abrir la conexion, en caso de una falla reintento luego de 1 seg

	do {
		conexion = crear_conexion( ip, puerto );
		if (conexion == -1) //esperar un seg para reintentar
			sleep(1);
	}while(conexion == -1);
	//printf("Enviando a socket %d...\n",conexion);

	enviar_paquete(paquete, conexion);	//Envio el paquete
	return conexion;
}

void eliminar_paquete(t_paquete* paquete)
{
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

void liberar_conexion(int socket_cliente)
{
	close(socket_cliente);
}

void agregar_entero_a_paquete(t_paquete* paquete, int x) // Agrega un entero a un paquete (ya creado)
{
	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(int));
	
	memcpy(paquete->buffer->stream + paquete->buffer->size, &x, sizeof(int));

	paquete->buffer->size += sizeof(int);
	
}


/*
void agregar_opCode_a_paquete(t_paquete* paquete, op_codeTripu x) // Agrega un entero a un paquete (ya creado)
{
	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(op_codeTripu));
	
	memcpy(paquete->buffer->stream + paquete->buffer->size, &x, sizeof(op_codeTripu));

	paquete->buffer->size += sizeof(op_codeTripu);
	
}*/

void agregar_array_string_a_paquete(t_paquete* paquete, char** arr)
{
	int size = string_arr_size(arr);
	agregar_entero_a_paquete(paquete,size);
	for(int i = 0; i < size; i++)
		agregar_a_paquete(paquete, arr[i], strlen(arr[i])+1);
}

int iniciar_servidor(char* ip, char* puerto)
{
	int socket_servidor;

    struct addrinfo hints, *servinfo, *p;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(ip, puerto, &hints, &servinfo);

    for (p=servinfo; p != NULL; p = p->ai_next)
    {
        if ((socket_servidor = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
            continue;

        if (bind(socket_servidor, p->ai_addr, p->ai_addrlen) == -1) {
            close(socket_servidor);
            continue;
        }
        break;
    }

	listen(socket_servidor, SOMAXCONN);

    freeaddrinfo(servinfo);

    //log_trace(logger, "Listo para escuchar a mi cliente");

    return socket_servidor;
}

int esperar_cliente(int socket_servidor)
{
	struct sockaddr_in dir_cliente;
	unsigned int tam_direccion = sizeof(struct sockaddr_in);

	
	int socket_cliente = accept(socket_servidor, (void*) &dir_cliente, &tam_direccion);
	
	
	return socket_cliente;
}



int recibir_operacion(int socket_cliente)
{
	int cod_op;
	if(recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) != 0)
		return cod_op;
	else
	{
		close(socket_cliente);
		return -1;
	}

}

void* recibir_buffer(int* size, int socket_cliente)
{
	void * buffer;

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
}

void recibir_mensaje(int socket_cliente)
{
	int size;
	char* buffer = recibir_buffer(&size, socket_cliente);
	printf("Me llego el mensaje %s", buffer);
	//log_info(logger, "Me llego el mensaje %s", buffer);
	free(buffer);
}

void recibir_mensaje_bitacora(int socket_cliente)
{
	int size;
	char* buffer = recibir_buffer(&size, socket_cliente);
	printf("Me llego la bitacora: %s \n", buffer);
	//log_info(logger, "Me llego el mensaje %s", buffer);
	free(buffer);
}

//podemos usar la lista de valores para poder hablar del for y de como recorrer la lista
t_list* recibir_paquete(int socket_cliente)
{
	int size;
	int desplazamiento = 0;
	void * buffer;
	t_list* valores = list_create();
	int tamanio;

	buffer = recibir_buffer(&size, socket_cliente);
	while(desplazamiento < size)
	{
		memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);
		char* valor = malloc(tamanio);
		memcpy(valor, buffer+desplazamiento, tamanio);
		desplazamiento+=tamanio;
		list_add(valores, valor);
	}
	free(buffer);
	return valores;
	return NULL;
}

int leer_entero(char*buffer, int* desplazamiento)
{
	int ret;
	memcpy(&ret, buffer + (*desplazamiento), sizeof(int));
	(*desplazamiento)+=sizeof(int);
	return ret;
}

char** leer_array_string(char*buffer, int* desp)
{
	int len = leer_entero(buffer, desp);
	char** arr = malloc((len+1)*sizeof(char*));

	for(int i = 0; i < len; i++)
	{
		arr[i] = leer_string(buffer, desp);
	}
	arr[len] = NULL;

	return arr;
}
//Leo un string del buffer, dejo en desp lo que ya lei
char* leer_string(char* buffer, int* desplazamiento)
{
	//char* buf = (char*) buffer;

	int tamanio = leer_entero(buffer, desplazamiento);
	//printf("allocating / copying %d \n",tamanio);
	/*

	memcpy(&tamanio, buffer + (*desplazamiento), sizeof(int));
	(*desplazamiento)+=sizeof(int);
	*/
	char* valor = malloc(tamanio);
	memcpy(valor, buffer+(*desplazamiento), tamanio);
	(*desplazamiento)+=tamanio;

	return valor;
}

int* leer_array_entero(char* buffer, int* desp)
{
	int len = leer_entero(buffer, desp);

	int* arr = malloc((len+1)*sizeof(int));
	for(int i = 0; i < len; i++)
	{
		arr[i] = leer_entero(buffer, desp);
	}
	arr[len] = -1;

	return arr;
}


int stringArraySize(char** aux){
	
	int i = 0;
	while(aux[i] != NULL)
	{
		i += sizeof(aux[i]);
	}
	return i;

}

void enviar_codop(op_code codop, int socket)
{
	t_paquete* paquete = crear_paquete();
	paquete->codigo_operacion = codop;
	enviar_paquete(paquete, socket);
	free(paquete);
}

void enviarFail(int socket){
	
	t_paquete* paquete;
		
	paquete = crear_paquete_con_codigo_op(FAIL);
	
	enviar_paquete(paquete,socket);
	
	eliminar_paquete(paquete);
	
	liberar_conexion(socket);
	
}

void enviarOK(int socket){
	
	pthread_mutex_lock(&mutexOk);
	
	t_paquete* paquete;
						
	paquete = crear_paquete_con_codigo_op(OK);
			
	enviar_paquete(paquete,socket);
	
	eliminar_paquete(paquete);
	
	pthread_mutex_unlock(&mutexOk);
	//liberar_conexion(socket);
	
}

void liberarStringArray(char** stringArray){
	
	int size = tamanioStringArray(stringArray);
	
	for(int i = 0; i< size; i++){
		
		free(stringArray[i]);
		
		
	}
	
	free(stringArray);
	
}

int tamanioStringArray(char** a)
{	
	int i = 0;
	while(a[i] != NULL)
	{
		i++;
	}
	return i;
}
