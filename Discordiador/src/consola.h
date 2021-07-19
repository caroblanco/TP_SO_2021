#ifndef CONSOLA_DISCORDIADOR_H_
#define CONSOLA_DISCORDIADOR_H_

#include "tripulante.h"
#include "discordiador.h"
#include <pthread.h>


void manejarConsola(void);
char** crearPosiciones(char** , int , int );
void imprimirTripulante(tripulante * );
char* leerArchivo(char* );
//

#endif
