#include "opComunes.h"

int elegirEsquema(){ 
	
	int esquema;
	
	if(string_equals_ignore_case(config_valores.esquema_memoria,"PAGINACION")){
        esquema = PAGINACION;
    }else if(string_equals_ignore_case(config_valores.esquema_memoria, "SEGMENTACION")){
        esquema = SEGMENTACION; 
    }
	
	return esquema;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////INICIAR PATOTA
void iniciarPatota(int socket_cliente){
    int size;
	char * buffer;
	int desp = 0;

	buffer = recibir_buffer(&size, socket_cliente);
		
	char* tareas = leer_string(buffer, &desp);	//STRING DE TODAS LAS TAREAS \n
	//printf("lei el string de las tareas");
		
	int idPatota = leer_entero(buffer, &desp);
	//printf("lei el id de una patota: %d", idPatota);

	int cantTripus = leer_entero(buffer, &desp);
	//printf("lei la cantidad de tripus: %d", cantTripus);
	
	int tamanioTotal = 21 * cantTripus + (strlen(tareas)+1) + 8;
	
	log_info(logger, "Iniciando patota %d con %d tripulantes que pesa %d...", idPatota, cantTripus, tamanioTotal);
	

	int pudeGuardar;
	
	if(elegirEsquema() == SEGMENTACION){            
        pudeGuardar = s_iniciarPatota(cantTripus, idPatota, tareas, tamanioTotal);               //DEVUELVE EL PAQUETE OK O FAIL
    }else if(elegirEsquema() == PAGINACION){
        pudeGuardar = p_iniciarPatota(cantTripus, idPatota, tareas, tamanioTotal); 
    }
	
	if(pudeGuardar == 1){
		enviarOK(socket_cliente);
		log_info(logger,"Envio ok");
	}
	else
	{	
		log_error(logger, "No hay lugar para alojar la patota en memoria, lo sentimos");
		enviarFail(socket_cliente);
	}

	free(buffer);
	free(tareas);
}

int s_iniciarPatota(int cantTripus, int idPatota, char* tareas, int tamanioTotal){
    
    //HAYLUGAR -> UN TCB = 21 BYTES, VAN A HABER N TCBS + 1 PCB (OCUPA 8 BYTES) + TAREAS
    
    if(puedoGuardar(tamanioTotal)){
		//AGREGAR LAS TAREAS A LA MEMORIA -> T_SEGMENTO DE TAREAS
		//t_segmento* segmentoTareas = guardarTareasSeg(tareasSeparadas); //DEVUELVE EL SEGMENTO DONDE SE GUARDO
		t_segmento* segmentoTareas = guardarTareasSeg(tareas); //MANDO EL STRING DE LAS TAREAS NO LAS TAREAS SEPARADAS
		
		t_pcb* pcb = malloc(sizeof(t_pcb));
		pcb->idPatota = idPatota;
		pcb->tareas = segmentoTareas->base;	//DONDE INICIAN LAS TAREAS

		//GUARDO LA PCB EN MEMORIA -> T_SEGMENTO DE PATOTAS
		t_segmento* pcb_seg = guardarPcbSeg(pcb);//DEVUELVE EL SEGMENTO DONDE SE GUARDO
		//printf("agregue la patota");
		
		// CREO LA TABLA DE LA PATOTA Y GUARDO EL SEGMENTO DE LAS TAREAS Y EL DEL PCB, GUARDO EN LA LISTA GLOBAL DE PATOTAS
		t_tabla_patota* tablaPCB = crearTablaPatota(idPatota, pcb_seg, segmentoTareas);
		
		pthread_mutex_lock(&mutexListaDePatotas);
		list_add(tablaDeSegmentosDePatotas,tablaPCB);//Falta sincronizar
		pthread_mutex_unlock(&mutexListaDePatotas);
		
		free(pcb);
		
		
		return 1;

	}else{

		return 0;
	}
}

int p_iniciarPatota(int cantTripus, int idPatota, char* tareas, int tamanioTotal){
    //HAYLUGAR -> UN TCB = 21 BYTES, VAN A HABER N TCBS + 1 PCB (OCUPA 8 BYTES) + TAREAS
	int paginasNecesarias = ceil((double) tamanioTotal/ (double) config_valores.tamanio_pagina);
	
	int tamanioTareas = (strlen(tareas) + 1);
	
	if(puedoGuardarPaginacion(paginasNecesarias)){		//SI HAY LUGAR EN MP O MV	

        //PRIMERO GUARDAMOS LAS TAREAS 
        t_list* paginasTareas = guardarTareasPaginacion(tareas);
        
        void mostrarId(t_pagina* pag)
        {
        	log_info(logger, "%d",pag->id);
        }
        
        log_info(logger, "Guarde las tareas de la patota %d en las paginas: ",idPatota);
        list_map(paginasTareas, (void*)mostrarId);

        //DIRECCION LOGICA DE LAS TAREAS PARA GUARDARLA EN LA PCB
        t_pagina* primerPagina= list_get(paginasTareas,0); 
        int dirTareas= calcularDirLogica(primerPagina,0);

        //CREAMOS LA TABLA DE PAGINAS
        t_tabla_pagina* unaTabla = crearTablaDePaginas(idPatota, tamanioTareas, cantTripus);

		//GUARDO LAS TAREAS EN LA TABLA DE LA PATOTA CON EL ID PATOTA
        agregarPaginasATablaDePatota(unaTabla, paginasTareas);
		
        //CREAMOS LE PCB
        t_pcb* pcb = malloc(sizeof(t_pcb));
		pcb->idPatota = idPatota;
		pcb->tareas = dirTareas; //DONDE INICIAN LAS TAREAS

        //GUARDAMOS LA PCB
        guardarPcbPaginacion(pcb);


        free(pcb);

        return 1;

	}else{
		return 0;
	}

}

////////////////////////////////////////////////////////////////////////////////////////INICIAR TRIPULANTE
void iniciarTripulante(int socket_cliente){
    int size;
	char * buffer;
	int desp = 0;

	t_tcb* tripulanteNuevo = malloc(sizeof(t_tcb));

	buffer = recibir_buffer(&size, socket_cliente);
		
	int idTripu = leer_entero(buffer, &desp);
	//printf("lei el id de un tripu: %d", idTripu);

	int idPato = leer_entero(buffer, &desp);
	//printf("lei el id de una patota: %d", idPato);
		
	int posX = leer_entero(buffer, &desp);
	//printf("lei la pos_x de un tripulante: %d", posX);
		
	int posY = leer_entero(buffer, &desp);
	//printf("lei la pos en Y del tripu: %d", posY);
	
	log_info(logger, "Iniciando tripulante %d de la patota %d en la posicion %d|%d", idTripu, idPato, posX, posY);

	tripulanteNuevo->idTripulante 	 = idTripu;
	tripulanteNuevo->estado			 = 'n';
	tripulanteNuevo->posX 			 = posX;
	tripulanteNuevo->posY 			 = posY;
	tripulanteNuevo->proxInstruccion = 0;
	

	//AGREGO AL TRIPULANTE AL MAPA
	char charId = intToChar(idTripu);
	dibujar_tripulante(charId,  posX,  posY);


    if(elegirEsquema() == SEGMENTACION){
        s_iniciarTripulante(tripulanteNuevo, idPato);                                     
    }else if(elegirEsquema() == PAGINACION){
        p_iniciarTripulante(tripulanteNuevo, idPato);
    }
	//log_info(logger, "Guarde la tcb de %d en memoria", tripulanteNuevo->idTripulante);
	free(buffer);
	free(tripulanteNuevo);
	liberar_conexion(socket_cliente);
}

void s_iniciarTripulante(t_tcb* tripulanteNuevo,int idPato){


    tripulanteNuevo->puntero_pcb = idPato;
    
    //GUARDO LA TCB EN MEMORIA
	t_segmento* tcb_seg = guardarTcbSeg(tripulanteNuevo);
	
	//Sumo un tripu vivo a la tabla de patota
	
	sumarTripu(idPato);

	//GUARDO LA TCB EN LA LISTA GLOBAL DE TCB
	t_tabla_tripulante* tablaTCB = crearTablaTripulante(tripulanteNuevo->idTripulante, tcb_seg);
	
	pthread_mutex_lock(&mutexListaDeTripulantes);
	list_add(tablaDeSegmentosDeTripulantes, tablaTCB);
	pthread_mutex_unlock(&mutexListaDeTripulantes);


}

void p_iniciarTripulante(t_tcb* tripulanteNuevo, int idPatota){
	
	
	//buscamos la patota
	int direccionPatota = buscarDirPatotaPag(idPatota);
		
	tripulanteNuevo->puntero_pcb	= direccionPatota;	// ES DIR LOGICA
	

	pthread_mutex_lock(&mutexSwap);
	
	//AGREGAMOS EL ID A LA TABLA DE PAGINA DE LA PATOTA
	agregarIdTripulanteAPag(tripulanteNuevo->idTripulante,idPatota);

	//GUARDAMOS EL TCB EN UNA PAGINA
	guardarTcbPaginacion(tripulanteNuevo, idPatota);
	
	pthread_mutex_unlock(&mutexSwap);

}
/////////////////////////////////////////////////////////////////////////////EXPULSAR TRIPULANTE

void expulsarTripulante(int socket_cliente){
	int size;
	char * buffer;
	int desp = 0;

	buffer = recibir_buffer(&size, socket_cliente);
	
	int idTripu = leer_entero(buffer, &desp);
	//printf("lei el id de un tripulante: %d", idTripu);
		
	
	expulsarTripulanteID(idTripu);


	free(buffer);
}

void s_expulsarTripulante(int idTripu){
	
	t_tcb* unaTcb = buscarTcb(idTripu);
	int idPato = unaTcb->puntero_pcb;
    
	//LO SACAMOS DE LA MEMORIA -> CAMBIAMOS EL BITMAP
	eliminarTcbSeg(idTripu);

	//LO SACAMOS DE LA LISTA GLOBAL DE TCB
	sacarTripulanteListaGlobal(idTripu);
	
	log_info(logger,"Se expulso el tripulante %d", idTripu);
	
	//Resto de la tabla de su patota
	
	
	restarTripu(idPato);
	
	free(unaTcb);
	
}

void sacarTripulanteListaGlobal(int idTripulanteAEliminar){


	int mismo_id(t_tabla_tripulante* unaTable){
		return ((unaTable->idTripulante) == idTripulanteAEliminar );
	}
	//pthread_mutex_lock(&mutexListaDeTripulantes);
	list_remove_and_destroy_by_condition(tablaDeSegmentosDeTripulantes, (void*) mismo_id, (void*) liberarTabla );
	//pthread_mutex_unlock(&mutexListaDeTripulantes);
}



/////////////////////////////////////////////////////////////////////////////////ACTUALIZAR UBICACION
void actualizarUbicacion(int socket_cliente){
	int size;
	char * buffer;
	int desp = 0;

	buffer = recibir_buffer(&size, socket_cliente);
	
	int tid = leer_entero(buffer, &desp);
	//printf("lei el id de un tripulante: %d", id);
	
	int pid = leer_entero(buffer, &desp);//para saltear el id de la patota
	pid++;
	
	int nuevaPosX = leer_entero(buffer, &desp);
	//log_info(logger,"lei la nueva posicion en X del tripulante: %d", nuevaPosX);
	
	int nuevaPosY = leer_entero(buffer, &desp);
	//log_info(logger,"lei la nueva posicion en Y del tripulante: %d", nuevaPosY);

	t_tcb* nuevaTcb;
	
	log_info(logger, "El tripulante %d se movio a la posicion %d|%d",tid, nuevaPosX, nuevaPosY);
	
	nuevaTcb = buscarTcb(tid);

	nuevaTcb->posX = nuevaPosX;
	nuevaTcb->posY = nuevaPosY;
	
	actualizarTripulante(nuevaTcb);
	
	//PASAR EL ID A CHAR
	char charId = intToChar(tid);
	moverTripulante(charId,nuevaPosX,nuevaPosY); 
	
	enviarOK(socket_cliente);
	free(buffer);
	//liberar_conexion(socket_cliente);
}

//TOTO ES EL MEJOR

t_tcb* buscarTcb(int tid){
	
	t_tcb* nuevaTcb;
	
	if(elegirEsquema() == SEGMENTACION){
		nuevaTcb = buscarTripulante(tid);
	
	}else if(elegirEsquema() == PAGINACION){
		nuevaTcb = p_buscarTripulante(tid);
	
	}
	
	return nuevaTcb;
	
}

void actualizarTripulante(t_tcb* tcb){
	
	
	if(elegirEsquema() == SEGMENTACION){
		s_actualizarTripulante(tcb);
	}else if(elegirEsquema() == PAGINACION){
		p_actualizarTripulante(tcb);
	}
	
	free(tcb);
}

char* buscarTareaEsquema(t_tcb* tcb,int pid, int* esUltima){
	char* tarea;
    if(elegirEsquema() == SEGMENTACION){       
		tarea = s_enviarOp(tcb, esUltima);      
    }else if(elegirEsquema() == PAGINACION){
		tarea = p_enviarOp(tcb, pid, esUltima);
    }
	
    return tarea;
}

void expulsarTripulanteID(int idTripu){
	
	//LO SACO DEL MAPA
	char charId = intToChar(idTripu);
	expulsarDelMapa(charId);
	
    if(elegirEsquema() == SEGMENTACION){ 
        s_expulsarTripulante(idTripu);                                     
    }else if(elegirEsquema() == PAGINACION){
        p_expulsarTripulante(idTripu);		
    }

}

//////////////////////////////////////////////////////////////////////////////////////////ENVIAR OPERACION

void enviarOperacion(int socket_cliente){
    int size;
	char * buffer;
	int desp = 0;
	t_tcb* tcb;

	
	char* tarea;
	int esUltima;

	
	buffer = recibir_buffer(&size, socket_cliente);
	
	int idPatota = leer_entero(buffer, &desp);
	//printf("lei el id de una patota: %d", idPatota);

    int idTripu = leer_entero(buffer, &desp);
	//printf("lei el id de un tripulante: %d", idTripu);
    

    
    tcb = buscarTcb(idTripu);
    tarea = buscarTareaEsquema(tcb, idPatota, &esUltima);
    
    log_info(logger, "La proxima tarea del tripulante %d es %s ",idTripu, tarea);

    if(tarea != NULL)
    {
		//ACTUALIZO EL TRIPULANTE
		tcb->proxInstruccion ++;
		tcb-> estado = 'r';
	
		actualizarTripulante(tcb);
		

		
		//CREO PAQUETE
			t_paquete* nuevoPaquete = armarPaquete(tarea, esUltima);//no se de donde sale la tarea, esLaUltimaTarea tendria que ser, buscarProx tarea y le pasamos un int que diga si es la ultima o no
		//ENVIAR PAQUETE
			enviar_paquete(nuevoPaquete, socket_cliente);
		//BORRAR PAQUETE
			eliminar_paquete(nuevoPaquete);
		//LIBERAR CONEXION
			//liberar_conexion(socket_cliente);    	
    }
    else
    {
    	enviarFail(socket_cliente);
    	free(tcb);
    }

	free(buffer);
	free(tarea);
	
}

char* s_enviarOp(t_tcb* tcb, int* esUltima){


	//BUSCAMOS LA TAREA USANDO el PID
	char* tarea = buscarTarea(tcb->puntero_pcb, tcb->proxInstruccion, esUltima);

	return tarea;
}

char* p_enviarOp(t_tcb* tcb,int idPatota, int* esUltima){
	//BUSCAMOS LA TAREA USANDO LA LISTA DE TAREAS DE LA PCB Y LA PROX INSTRUCCION DE LA TCB
	char* tarea = buscarTareaPaginacion(idPatota, tcb->proxInstruccion, esUltima);

	return tarea;
}

t_paquete* armarPaquete(char* tarea, int esLaUltima){
	
	//SACAMOS LA INFO DE LA TAREA
	char** splitPorPuntoYcoma = string_split(tarea, ";"); 						// SEPARA CODIGO PARAMETROS ; POSX ; POSY ; TIEMPO
	char** splitPorEspacio = string_split(splitPorPuntoYcoma[0], " "); 			// SEPARA CODIGO PARAMETROS
	 
	//PASAMOS LA INFO DE LA TAREA A INTS
	op_code codigoOperacion;
	if(			string_equals_ignore_case(splitPorEspacio[0], "GENERAR_OXIGENO")) { codigoOperacion= GENERAR_OXIGENO;	}
	else if(	string_equals_ignore_case(splitPorEspacio[0], "CONSUMIR_OXIGENO")){ codigoOperacion= CONSUMIR_OXIGENO;	}
	else if(	string_equals_ignore_case(splitPorEspacio[0], "GENERAR_COMIDA"))  { codigoOperacion= GENERAR_COMIDA;	}
	else if(	string_equals_ignore_case(splitPorEspacio[0], "CONSUMIR_COMIDA")) { codigoOperacion= CONSUMIR_COMIDA;	}
	else if(	string_equals_ignore_case(splitPorEspacio[0], "GENERAR_BASURA"))  { codigoOperacion= GENERAR_BASURA;	}
	else if(	string_equals_ignore_case(splitPorEspacio[0], "DESCARTAR_BASURA")){ codigoOperacion= DESCARTAR_BASURA;	}
	else                                                                          { codigoOperacion= MOVERSE;           }
	
	int posX 				= atoi(splitPorPuntoYcoma[1]);
	int posY 				= atoi(splitPorPuntoYcoma[2]);
	
	int parametros;
	if(splitPorEspacio[1] != NULL){
		parametros 			= atoi(splitPorEspacio[1]);		
	}
	else{
		parametros = 0;
	}
	
	int tiempo 				= atoi(splitPorPuntoYcoma[3]);
	
	log_info(logger,"Tengo la tarea %s, posX: %d posY: %d parametros: %d, tiempo:%d ult: %d",splitPorEspacio[0],posX,posY,parametros,tiempo,esLaUltima);

	//CREAMOS EL PAQUETE
	t_paquete* nuevoPaquete = crear_paquete_con_codigo_op(codigoOperacion);
	agregar_entero_a_paquete(nuevoPaquete, posX); 
	agregar_entero_a_paquete(nuevoPaquete, posY);
	agregar_entero_a_paquete(nuevoPaquete, parametros); 
	agregar_entero_a_paquete(nuevoPaquete, tiempo);
	agregar_entero_a_paquete(nuevoPaquete, esLaUltima);

	liberarStringArray(splitPorPuntoYcoma);
	liberarStringArray(splitPorEspacio);

	return nuevoPaquete;
}

///////////////////////////////////////////////////////////////////////////////////////////////////CAMBIAR ESTADO
void cambiarEstado(int socket_cliente){

	int size;
	char * buffer;
	int desp = 0;
	
	buffer = recibir_buffer(&size, socket_cliente);

	int idTripu = leer_entero(buffer, &desp);
	//printf("lei el id de un tripulante: %d", idTripu);

	//int idPatota = leer_entero(buffer, &desp);//ESTA AL PEDO PERO NO LO SACAMOS PORQUE EL ESTADO VIENE DESPUES Y SINO CAMBIA EL DESPLAZAMIENTO
	//printf("lei el id de una patota: %d", idPatota);
	
	char* nuevoEstado = leer_string(buffer, &desp);	
	//printf("lei el nuevo estado");
	
	log_info(logger, "El nuevo estado del tripulante %d es %c", idTripu, nuevoEstado[0]);
	
	if(string_equals_ignore_case(nuevoEstado, "f"))
	{
		expulsarTripulanteID(idTripu);
	}
	else
	{
		t_tcb* tcb = buscarTcb(idTripu);
		
		tcb->estado = nuevoEstado[0];
		
		actualizarTripulante(tcb);			
	}
	enviarOK(socket_cliente);
	log_info(logger,"Envio ok");
	//liberar_conexion(socket_cliente);
	free(buffer);
	free(nuevoEstado);
}
