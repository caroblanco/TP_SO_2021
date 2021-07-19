
#ifndef PLANITIFACODR_DISCORDIADOR_H_
#define PLANITIFACODR_DISCORDIADOR_H_


#include "discordiador.h"


#include<commons/collections/list.h>

sem_t planificacion;
sem_t operacionesTripulantes;
sem_t planificacionPausada;
sem_t sincroTripuPlanif;
sem_t sabotaje;
sem_t sincroTripuPlanif2;
pthread_mutex_t mensaje_mongo;
pthread_mutex_t mutexRam;

void planificador_crearTripulantes(int  ,char*,int  );
void planificacionFIFO();
void planificacionRR();
//void agregarAReady(tripulante  );

void avisarARamCambio(int id,int id_patota,char* estado);

#endif
