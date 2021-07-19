#include "mapa.h"

//CREAMOS EL MAPA VACIO
void crearMapaVacio(){

    nivel_gui_inicializar();
    nivel_gui_get_area_nivel(&columnas, &filas);

    nivel = nivel_crear("Nave");
    nivel_gui_dibujar(nivel);
}

//Para manejar los errores del mapa
void ASSERT_CREATE(char id,int error){
    if(error) {
        nivel_destruir(nivel);
        nivel_gui_terminar();
        fprintf(stderr, "Error al crear '%c': %s\n", id, nivel_gui_string_error(error));
       // return EXIT_FAILURE;
    }
}

//Actualizamos la nave
void actualizar_Nivel(){
	
	pthread_mutex_lock(&mutexMapa);
    nivel_gui_dibujar(nivel);
    pthread_mutex_unlock(&mutexMapa);
    
}

//Cambiar al tripulandte de posicion
void moverTripulante(char id , int posX, int posY){

    item_mover(nivel, id, posX, posY);
    actualizar_Nivel();
}

//Sacar al tripulante 
void expulsarDelMapa(char unId){

    item_borrar(nivel,unId);
    actualizar_Nivel();
}

//Visualizar al tripulante en el mapa
void dibujar_tripulante( char id, int posX, int posY){

    error = personaje_crear(nivel,id, posX,posY);
    ASSERT_CREATE(id,error);
    actualizar_Nivel();
}

void finalizar_mapa(){
	//int nivel_destruir();   //ESTAN SEGURAS QUE ESTE METODO EXISTE?
	nivel_gui_terminar();
}
