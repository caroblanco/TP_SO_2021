#ifndef MAPA
#define MAPA

#include <utils.h>
#include <ncurses.h>
#include <nivel-gui/nivel-gui.h>
#include <nivel-gui/tad_nivel.h>

int error;
int columnas;
int filas;

pthread_mutex_t mutexMapa;

void ASSERT_CREATE(char id,int error);
void crearMapaVacio();
void actualizar_Nivel();
void moverTripulante(char id , int posX, int posY);
void dibujar_tripulante( char id, int posX, int posY);
void expulsarDelMapa(char unId);
void finalizar_mapa();


NIVEL* nivel;



#endif /*MAPA*/
