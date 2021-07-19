#ifndef MEMORIASEG_H_
#define MEMORIASEG_H_

#include"memoria.h"

//STRUCTS SEGMENTOS
typedef struct
{
    int id;
    int base;
    int limite;
}t_segmento;

typedef struct
{
    uint32_t idPatota;              // indica la patota
    t_segmento* pcb;
    t_segmento* tareas;
    int tripusVivos;
    //t_list* tablaSegmentosPatota;   // pcb | tareas
}t_tabla_patota;

typedef struct
{
    uint32_t idTripulante; //PARA DESPUES PODER SABER A QUE PATOTA PERTENECE
    t_segmento* seg_tcb;
}t_tabla_tripulante;


t_list* tablaDeSegmentosDePatotas; //GUARDA LAS PATOTAS
t_list* tablaDeSegmentosDeTripulantes;

char* datos;

t_bitarray* bitMapSegment;
t_bitarray* unBitmap;

pthread_mutex_t mutexBitMapSegment;
pthread_mutex_t mutexListaDeTripulantes;
pthread_mutex_t mutexListaDePatotas;
pthread_mutex_t mutexVivos;


int iniciarSegmentacion(void);
t_segmento* guardarTareasSeg(char* tareas);
t_segmento* guardarElemento(void*, int);
void guardarEnMemoria(void*, t_segmento*, int);
void ocuparMemoria(void*, int, int);
t_tabla_patota* crearTablaPatota(int, t_segmento*, t_segmento*);
int buscarDirPatota(int);
t_tabla_patota* buscarTablaPatota(int);
int puedoGuardar(int);
int tamanioTotalDisponible(void);
t_segmento* guardarPcbSeg(t_pcb*);
t_segmento* guardarTcbSeg(t_tcb*);
void eliminarTcbSeg(int);
void* copiarSegmentacion(t_segmento*);
t_tabla_tripulante* crearTablaTripulante(int , t_segmento* );
t_tabla_tripulante* buscarTablaTripulante(int );
t_tcb* buscarTripulante(int);
t_segmento* buscarSegmentoTripulante(int);
char* buscarTarea(int pid , int desplazamiento, int* esUltima);
char* buscarArrayTareas(int pid);
int esLaUltimaTarea(int, int);
int buscarDirPatota(int);
t_tabla_patota* buscarTablaPatota(int);
t_segmento* buscarSegmentoSegunTamanio(int);
t_list* puedenGuardar(t_list*, int );
t_list* buscarSegmentosDisponibles();
t_segmento* buscarUnLugarLibre(int*);
t_list* buscarSegmentosOcupados();
void agregarPatotasA(t_list*);
void agregarTripusA(t_list*);
t_segmento* elegirSegCriterio(t_list* , int );
t_segmento* segmentoBestFit(t_list* , int );
t_segmento* segmentoMenorTamanio(t_segmento* , t_segmento* );
void compactacion();
t_list* copiarContenidoSeg(t_list* );
void actualizarCompactacion(t_list* ,t_list*);
void actualizarCadaSegmento(t_segmento* , t_segmento* );
void actualizarListaPatotas(t_segmento* segmentoViejo, t_segmento* segmentoNuevo);
void actualizarListaTripulantes(t_segmento* segmentoViejo, t_segmento* segmentoNuevo);
void actualizarBase(t_segmento* , t_segmento* );
void s_actualizarTripulante(t_tcb* unaTcb);
void dumpSegmentacion();
void mostrarProcesosSegmentacion(FILE* file);

void mostrarProceso(t_tabla_patota* unaTabla, FILE* file);

void mostrarSegmentoDump(int pid, t_segmento* unSegmento, FILE* file);

void mostrarSegmentosTripulantes(int pid, FILE* file);

void liberarTabla(t_tabla_tripulante* unaTabla);

void mostrarTripulante(int id);

void liberarMemoriaSegmentacion();

void eliminarTablaPatota(t_tabla_patota* unaTabla);


void eliminarTablaTripulante(t_tabla_tripulante* unaTabla);

void sumarTripu(int idPato);

void restarTripu(int idPato);




#endif /*MEMORIASEG_H_*/
