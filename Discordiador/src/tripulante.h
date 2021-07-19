#ifndef TRIPULANTE_DISCORDIADOR_H_
#define TRIPULANTE_DISCORDIADOR_H_


#include "planificador.h"
#include "discordiador.h"
#include <pthread.h>
#include<commons/collections/list.h>



t_list* tripulantesNuevos; //LISTA DE TRIPULANTES
t_list* tripulantesReady;
t_list* tripulantesBlockIO;
t_list* tripulantesBlock;
t_list* tripulantesExecute;
t_list* tripulantesFinalizados;
t_list* bloqueadosEmergencia;
int puerto_inicial;

typedef struct{
	//pthread_t hilo;
	int pos_x;
	int pos_y;
	int id_patota;
	int id;
	int ciclos; //ciclos ejecutados
	int aEjectuar; //CICLOS TOTALES A EJECTUAR
	int pasosAdar;
	int tocaIO;
	int ultimaTarea;
	int ejecutado;
	int elegidoPorElDestino; // SI ESTA EN 0 NO TIENE QUE IR A SABOTAJE
	int noTaListo; // SE USA PARA CUANDO ESTA PIDIENDO LA TAREA, COMO QUE NO ESTA LISTO PARA SER PLANIFICADO 
	char estado;
	char estadoViejo;
	sem_t semaforoTripulante;
}tripulante;


void manejarConsola(void);
void iniciarPatota(int , int , int ,char** );
void pedirCodigoOperacionAMiRar(int  ,int  );
void funcionesTripulantes (tripulante* );
int pasosAdar(int ,int ,int , int );
void finTarea(int  ,int  );
void inicioTarea(int  ,int  );
void descartarBasura(int,int,int  );
void generarBasura(int,int,int  );
void consumirComida(int ,int,int );
void generarComida(int ,int,int );
void consumirOxigeno(int,int,int  );
void generarOxigeno(int,int,int );
void inicioMongo(int );
void finOxigeno(int  );
void finConsumirOxigeno(int  );
void finComida(int  );
void finConsumircomida(int  );
void finBasura(int  );
void finConsumirBasura(int  );
void finMongo(int  );
void realizarSabotaje(tripulante*  );
void realizarSabotaje2(tripulante*  );
void avisarleAmongo(int );


#endif
