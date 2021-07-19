#ifndef MEMORIAPAG_H_
#define MEMORIAPAG_H_

#include "memoria.h"

//STRUCTS PAGINACION
typedef struct
{
    int id;
    int frame_ppal;
    int frame_virtual;
    int presencia;
    int uso;
    int modificado;
    int lock;
    int tiempo_uso;
    int tamanioDisponible;
    int fragInterna;
}t_pagina;
typedef struct
{
    int idPatota;
    int tamanioTareas;
    int direPcb;
    int cantTripus;
    t_list* idTripus;
    t_list* paginas;
}t_tabla_pagina;

typedef struct
{
    int id;

}t_frame;

double tiempo;

char* data;

char* data2;

t_bitarray* frames_ocupados_virtual;
t_bitarray* frames_ocupados_ppal;
char* memoriaVirtualPath;
int cant_frames_ppal;
int cant_frames_virtual;
int posClock;

t_list* tablaDePaginas;


pthread_mutex_t mutexListaTablas;
pthread_mutex_t mutexBitmapMP;
pthread_mutex_t mutexBitmapMV;
pthread_mutex_t mutexMV;
pthread_mutex_t mutexSwap;
pthread_mutex_t mutexActualizar;
pthread_mutex_t mutexEliminar;
pthread_mutex_t mutexMostrar;
pthread_mutex_t mutexPags;
pthread_mutex_t mutexTiempo;
pthread_mutex_t mutexCopiarPags;
pthread_mutex_t mutexTripuLista;





void setLock(t_pagina* unaPag);
void cleanLock(t_pagina* unaPag);

void setModificado(t_pagina* unaPag);

int enMP(t_pagina* pag);

int enMV(t_pagina* pag);


////////////////////////////////////////////////////////////////PAGINACION///////////////////////////////////////////////////////
int iniciarPaginacion(void);

//GUARDAR EN MEMORIA

int puedoGuardarPaginacion(int paginasNecesarias);


t_list* guardarTareasPaginacion(char* tareas);
void guardarPcbPaginacion(t_pcb* pcb);

t_pagina* guardarAlgo(void* algo, int size, t_tabla_pagina* tabla, int* desplazamiento);



t_list* guardarElementoPaginacion(void* algo, int tamanio);

void guardarEnMemoriaPaginacion(void* algo, int tamanio,t_frame* unFrame, int mem);

void ocuparMemoriaPaginacion(void* algo, int tamanio, int idFrame, int desplazamiento, int mem);

void agregarPaginasATablaDePatota(t_tabla_pagina* unaTabla, t_list* paginasTareas);

void agregarPaginasATabla(t_tabla_pagina* unaTabla, t_list* paginas);
void agregarIdTripulanteAPag(int idTripu,int idPato);

void guardarTcbPaginacion(t_tcb* tcb, int idPato);

t_list* completarEspacioEnPagina(void* estructura, t_pagina* paginaAOcupar, int size, int* desplazamiento);

void* sacarInfoPaginas(t_tabla_pagina* unaTabla);

void copiarTodasLasPaginas(t_list* paginas, void* infoProceso);

void* copiarPaginasMP(t_list* paginas);
void copiarPaginas(t_list* listaDePaginas, void* algo);

//NACHO EXPLICANOS
void copiarPaginaDesp(t_pagina* unaPagina, void* algo, int* desplazamiento);

void* copiarPagina(t_pagina* pagina, int memoria);

t_tcb* sacarTripuPaginacion(int posTripuBuscado, void* losTripus);

//GUARDAR EN MV
void guardarMemoriaVirtual(t_pagina* pagina);
t_list* buscarPaginas(int nPrimerPagina, int size, t_tabla_pagina* unaTabla, int relleno);

void agregarPaginasALista(t_list* lista, t_list* paginas);
char* buscarTareaPaginacion(int idPatota, int proxInst, int* esUltima);

char* buscarTareasPaginacion(int idPatota);


t_tcb* buscarTripulantePaginacion(int idTripu, int idPatota);

t_tcb* copiarTcbPag(void* chocloDeTripus, int desplazamiento);

int buscarPosicionTripuEnLista(int idTripu, t_list* IdTripus);

t_tabla_pagina* buscarTablaPagina(int pid);

t_tabla_pagina* buscarTablaConTidPag(int tid);

int buscarDirPatotaPag(int idPato);

t_list* buscarFramesSinOcupar(int mem);

t_list* buscarPaginasDisponiblesEn(t_tabla_pagina* tabla);

int hayLugarEnLasPaginas(t_tabla_pagina* tabla);

t_tabla_pagina* buscarTablaPagConTid(int idTripu);

int tieneTripu(t_tabla_pagina* unaTabla, int idTripu);

t_tcb*  p_buscarTripulante(int idTripu);


t_list* buscarPaginasMP();

int memoriaDisponiblePag(int mem);
//CREAR
t_tabla_pagina* crearTablaDePaginas(int idPatota, int size, int cantTripus);

t_pagina* crearPagina(t_frame* frame, int espacioDisp);

int generarId();
int obtenerTiempo();

//ACTUALIZAR
void p_actualizarTripulante(t_tcb* nuevaTcb);

void sobreEscribirTcbPag(t_tcb* tcb, t_tabla_pagina* unaTabla);

int posTripuLista(int tid,t_list* tripIds);

t_list* buscarFramesDePaginas(t_list* paginas);

void actualizarElementoPag(void* algo, t_list* frames,  int size, int desplazamiento);

//EXPULSAR
void p_expulsarTripulante(int idTripu);

void eliminarPatota(t_tabla_pagina* unaTabla);
void matarPagina(t_pagina* unaPagina);

void liberarPaginas(t_list* paginas, int size, t_tabla_pagina* unaTabla, int base, int relleno);

void liberarPagina(t_pagina* unaPagina, t_tabla_pagina* unaTabla);

void liberarMemoriaPaginacion();

void eliminarTablaPaginas(t_tabla_pagina* unaTabla);


//SWAP
void swap(int cantPag);

void reemplazarPagina();

//LRU
void reemplazarConLRU(t_list* paginasEnMp);

//CLOCK
void reemplazarConClock(t_list* paginasEnMp);

t_pagina* elegirPaginaClock(t_list* paginas);

void revivirPaginas(t_list* paginasEnMV);
void revivirPagina(t_pagina* unaPagina);	


void ocuparFrame(t_frame* unFrame, int mem);

void desocuparFrame(int frame, int mem);

int calcularDirLogica(t_pagina* pagina, int desplazamiento);

//MMAP

void write_mmap(void* algo,int size,int base);

void read_mmap(int base, void* algo);

//DUMP

void dumpPaginacion();
void mostrarMarcosPaginacion(FILE* file);
void mostrarFrame(int nFrame, FILE* file);
t_pagina* buscarPaginaConFrame(int nFrame);
int buscarIdPatota(t_pagina* pagina);
void mostrarListaTripulantesPag(t_list* idLista);
void mostrarTripulantePag(int tid);

void iniciarSwap();

int existeArchivo(char* path);

void mostrarPagina(t_pagina* pagina);



#endif /*MEMORIAPAG_H_*/
