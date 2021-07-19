#include "mPaginacion.h"

void setLock(t_pagina* unaPag){
	
	unaPag->lock= 1;	
}
void cleanLock(t_pagina* unaPag){
	
	unaPag->lock= 0;
	
}

void setModificado(t_pagina* unaPag){
	
	unaPag->modificado = 1;
	
}

int enMP(t_pagina* pag){
	return (pag->presencia == 1);
}

int enMV(t_pagina* pag){
	return (pag->presencia == 0);
}



////////////////////////////////////////////////////////////////PAGINACION///////////////////////////////////////////////////////
int iniciarPaginacion(void){ 
    
    //MEMORIA PRINCIPAL
    memoriaPrincipal = malloc(config_valores.tamanio_memoria);

    if(memoriaPrincipal == NULL){
        //NO SE RESERVO LA MEMORIA
    	perror("MALLOC FAIL!\n");
        return 0;
    }

    int tamanioPaginas = config_valores.tamanio_pagina; //ES IGUAL AL TAMANIO DE LOS FRAMES DE LA MEMORIA
    memoriaVirtualPath = config_valores.path_swap;
    
    log_info(logger, "SWAP: %d",config_valores.tamanio_swap);
    
    cant_frames_ppal = config_valores.tamanio_memoria / tamanioPaginas;
    
    log_info(logger,"Tengo %d marcos de %d bytes en memoria principal",cant_frames_ppal, config_valores.tamanio_pagina);
    
    data = asignarMemoriaBits(cant_frames_ppal);
    
    if(data == NULL){
        
        perror("MALLOC FAIL!\n");
        return 0;
    }

    memset(data,0,cant_frames_ppal/8);
    frames_ocupados_ppal = bitarray_create_with_mode(data, cant_frames_ppal/8, MSB_FIRST);
    
    //MEMORIA VIRTUAL
    int tamanioSwap = config_valores.tamanio_swap; 
    cant_frames_virtual = tamanioSwap / tamanioPaginas; 
    
    log_info(logger,"Tengo %d marcos de %d bytes en memoria virtual",cant_frames_virtual, config_valores.tamanio_pagina);
    
    data2 = asignarMemoriaBits(cant_frames_virtual);
    if(data2 == NULL){
        
        perror("MALLOC FAIL!\n");
        return 0;
    }
    
    frames_ocupados_virtual = bitarray_create_with_mode(data2, cant_frames_virtual/8, MSB_FIRST);

    tablaDePaginas = list_create();
    
    posClock=0;
    
    iniciarSwap();
    
    return 1;
}

//GUARDAR EN MEMORIA

int puedoGuardarPaginacion(int paginasNecesarias){
    
    int cantFramesLibresPpal = memoriaDisponiblePag(MEM_PPAL); 
    int cantFramesLibresVirtual = memoriaDisponiblePag(MEM_VIRT);

    if((cantFramesLibresPpal + cantFramesLibresVirtual) >= paginasNecesarias){
        return 1;
    }else{
        return 0;
    }
}


t_list* guardarTareasPaginacion(char* tareas){

    t_list* paginasQueOcupe;
    
    int tamanio = strlen(tareas)+1;

    paginasQueOcupe = guardarElementoPaginacion(tareas, tamanio);
    
    

    return paginasQueOcupe; //DEVUELVE LA O LAS PAGINAS EN DONDE SE GUARDARON LAS TAREAS   
}

void guardarPcbPaginacion(t_pcb* pcb){

    int desplazamiento=0;
    t_tabla_pagina* tabla = buscarTablaPagina(pcb->idPatota);
    t_pagina* pagina = guardarAlgo(pcb, 8, tabla, &desplazamiento);

    log_info(logger, "Guarde la pcb de la patota %d en las pagina %d y su desplazamiento es %d", tabla->idPatota, pagina->id, desplazamiento);
	
	tabla->direPcb = calcularDirLogica(pagina, desplazamiento);
}

t_pagina* guardarAlgo(void* algo, int size, t_tabla_pagina* tabla, int* desplazamiento){
	
    t_list* paginas;
    t_pagina* primeraPag;
    
    if(hayLugarEnLasPaginas(tabla)){ //VEMOS SI CUANDO GUARDAMOS LAS TAREAS SOBRO LUGAR EN LAS PAGINAS DE LA TABLA
        
        t_list* paginasDisp = buscarPaginasDisponiblesEn(tabla);
        primeraPag = list_get(paginasDisp,0);
        
        paginas =  completarEspacioEnPagina(algo,primeraPag, size, desplazamiento ); //GUARDO LA PCB EN EL LUGAR LIBRE DE LA PAG (8 BYTES)
        
        list_destroy(paginasDisp);

    }else{ //SI NO SOBRO ESPACIO EN NINGUNA PAGINA, CREO UNA NUEVA Y LA AGREGO A LA TABLA
    	
        paginas= guardarElementoPaginacion(algo, size);
        
        primeraPag = list_get(paginas,0);
    }
    

    if(!(list_is_empty(paginas))){ //SI SE CREO UNA PAGINA NUEVA 

        agregarPaginasATabla(tabla,paginas);
    }

    list_destroy(paginas);
	return primeraPag;
}



t_list* guardarElementoPaginacion(void* algo, int tamanio){ //GUARDA EN PAGINAS NUEVAS -> VACIAS 
    
    t_list* paginasQueOcupe = list_create();
    //BUSCO FRAMES LIBRES
    t_list* framesSinOcupar = buscarFramesSinOcupar(MEM_PPAL);

    int tamanioPag = config_valores.tamanio_pagina;

    if(list_is_empty(framesSinOcupar)){ //NO HAY FRAMES LIBRES EN MEMORIA PPAL -> SWAP
    	
    	
        //VEO LA CANT DE PAGINAS QUE NECESITO
    	
        int cantPag = ceil((double) tamanio / (double) config_valores.tamanio_pagina);
        
        //HAGO SWAP PARA ESA CANTIDAD
        
    	
        swap(cantPag);
        //VUELVO A ITERAR LA FUNCION CON EL SWAP HECHO
        
        t_list* nuevasPaginas = guardarElementoPaginacion(algo,tamanio);
		list_add_all(paginasQueOcupe, nuevasPaginas);
		list_destroy(nuevasPaginas);
        

    }else{ //HAY FRAMES LIBRES EN PPAL
        if(tamanio <= tamanioPag){   //SI LO QUE TENGO QUE GUARDAR ENTRA EN UNA PAGINA
        
            t_frame* frame = list_get(framesSinOcupar,0); // AGARRO EL PRIMERO QUE ENCUENTRO EN LA LISTA DE LIBRES
            guardarEnMemoriaPaginacion(algo, tamanio, frame,MEM_PPAL);
        
            t_pagina* pagina = crearPagina(frame, (tamanioPag - tamanio)); //SI LAS TAREAS OCUPAN MENOS QUE LA PAG ME VAN A SOBRAR BYTES
            list_add(paginasQueOcupe, pagina);
    }   
        else{  //SI NECESITO MAS PAGINAS
        	
            int sobrante = tamanio - tamanioPag;        //lo que no me entra en la primer pagina
        
            t_list* pagina = guardarElementoPaginacion(algo, tamanioPag);   //GUARDO LO PRIMERO
            t_list* restoPaginas = guardarElementoPaginacion(algo + tamanioPag, sobrante);
        
            list_add_all(paginasQueOcupe,pagina);
		    list_add_all(paginasQueOcupe,restoPaginas);

            list_destroy(pagina);
            list_destroy(restoPaginas);
        }
    }
    
    eliminarLista(framesSinOcupar);
    return paginasQueOcupe;
}

void guardarEnMemoriaPaginacion(void* algo, int tamanio,t_frame* unFrame, int mem){

    ocuparFrame(unFrame, mem);
    ocuparMemoriaPaginacion(algo, tamanio, unFrame->id, 0, mem); //LE PASO 0 PORQUE ES UNA PAGINA VACIA
    
}

void ocuparMemoriaPaginacion(void* algo, int tamanio, int idFrame, int desplazamiento, int mem){

    //BASE DEL FRAME = ID*TAMANIO_PAGINA + DESPLAZAMIENTO(dentro de la pagina) -> EL ID INDICA EL NRO DE FRAME

    int base = ((idFrame * config_valores.tamanio_pagina) + desplazamiento);
    
    if(mem == MEM_PPAL){
        pthread_mutex_lock(&mutexMemoria);
        memcpy(memoriaPrincipal + base, algo, tamanio);
        pthread_mutex_unlock(&mutexMemoria);
    }else if(mem == MEM_VIRT){
    	pthread_mutex_lock(&mutexMV);
		write_mmap(algo, tamanio, base);
		pthread_mutex_unlock(&mutexMV);
    }

}

void agregarPaginasATablaDePatota(t_tabla_pagina* unaTabla, t_list* paginasTareas){

	
	agregarPaginasATabla(unaTabla, paginasTareas);

    list_destroy(paginasTareas);
}

void agregarPaginasATabla(t_tabla_pagina* unaTabla, t_list* paginas){
    
    list_add_all(unaTabla->paginas, paginas);
}

void agregarIdTripulanteAPag(int idTripu,int idPato){

    t_tabla_pagina* unaTabla;
    unaTabla = buscarTablaPagina(idPato);

    int* id = malloc(sizeof(int)); 
	*id = idTripu;

    list_add(unaTabla->idTripus, id);
}

void guardarTcbPaginacion(t_tcb* tcb, int idPato){
    t_tabla_pagina* unaTabla;
    unaTabla = buscarTablaPagina(idPato);
    int desplazamiento = 0;
	
	t_pagina* primerPag = guardarAlgo(tcb, 21,unaTabla, &desplazamiento);
	
	log_info(logger, "Guarde al tripulante %d en la pagina %d y su desplazamiento es %d", tcb->idTripulante, primerPag->id, desplazamiento);
}

t_list* completarEspacioEnPagina(void* estructura, t_pagina* paginaAOcupar, int size, int* desplazamiento){
    t_list* paginasUsadas;

    if(paginaAOcupar->tamanioDisponible >= size){       //ENTRA EN LA PAGINA JUSTO O LE SOBRA ESPACIO
        int frame = paginaAOcupar->frame_ppal;
        *desplazamiento = config_valores.tamanio_pagina - paginaAOcupar->tamanioDisponible;   //EMPIEZO A OCUPAR A PARTIR DE ESTO

        ocuparMemoriaPaginacion(estructura, size, frame, *desplazamiento,MEM_PPAL);
    
        paginaAOcupar->tamanioDisponible -= size; //LE RESTO LO QUE ACABO DE OCUPAR
        
        paginasUsadas = list_create();
        
    }else{                                              //SI NO ENTRA EN LA PAGINA, OCUPO UN POOC DE ESA Y CREO UNA NUEVA
        int tamanioLibrePagina = paginaAOcupar->tamanioDisponible;

        t_list* nada = completarEspacioEnPagina(estructura, paginaAOcupar, tamanioLibrePagina, desplazamiento);    //RECURSIVA, LLENO LO QUE PUEDO
        list_destroy(nada);
        //GUARDO LO QUE QUEDA EN UNA PAGINA NUEVA

        paginasUsadas = guardarElementoPaginacion(estructura+tamanioLibrePagina, size-tamanioLibrePagina);
    }

    return paginasUsadas;
}

void* sacarInfoPaginas(t_tabla_pagina* unaTabla){
    void* infoProceso = malloc(config_valores.tamanio_pagina*list_size(unaTabla->paginas));

    copiarTodasLasPaginas(unaTabla->paginas, infoProceso);

    return infoProceso;
}

void copiarTodasLasPaginas(t_list* paginas, void* infoProceso){
	
    int desplazamiento=0;

    void copiarEnInfoProceso(t_pagina*unaPagina){
        copiarPaginaDesp(unaPagina, infoProceso, &desplazamiento);
    }
    
    list_iterate(paginas, (void*)copiarEnInfoProceso);
}

void* copiarPaginasMP(t_list* paginas){
	
	void* algo = malloc(config_valores.tamanio_pagina * list_size(paginas));
	
	//ME FIJO SI TODAS ESTAN EN MP
	if(list_all_satisfy(paginas, (void*)enMP)){ //SI TODAS ESTAN EN MP, LAS COPIO
		
        copiarPaginas(paginas, algo);
	}else{  //SI ALGUNA NO ESTA EN MP, LA ENGO QUE TRAER
		
		//LOCKEO TODAS LAS PAGS PARA QUE NINGUN OTRO PROCESO ME LAS SAQUE 
		list_iterate(paginas,(void*)setLock);
		
        //BUSCO LAS PAGINAS EN DISCO Y LAS REVIVO
		t_list* paginasEnMV = list_filter(paginas, (void*)enMV);
		
		
		
		revivirPaginas(paginasEnMV);
		
		
		
		//AHORA TODAS DEBERIAN ESTAR EN MP
		
		void* otroAlgo = copiarPaginasMP(paginas);
		

		memcpy(algo, otroAlgo, config_valores.tamanio_pagina * list_size(paginas));
		
		//DESLOCKEO TODAS LAS PAGS  
		list_iterate(paginas,(void*)cleanLock);
		
		free(otroAlgo);
	}
	return algo;
}


void copiarPaginas(t_list* listaDePaginas, void* algo){
	
	int desplazamiento = 0;
	
	void copiarEnAlgo(t_pagina* unaPagina){
		copiarPaginaDesp(unaPagina, algo, &desplazamiento);
	}
	
	list_iterate(listaDePaginas, (void*)copiarEnAlgo);	
}


void copiarPaginaDesp(t_pagina* unaPagina, void* algo, int* desplazamiento){
	
	int base = unaPagina->frame_ppal * config_valores.tamanio_pagina;
	
	pthread_mutex_lock(&mutexMemoria);
	memcpy(algo + *desplazamiento, memoriaPrincipal + base, config_valores.tamanio_pagina);
	pthread_mutex_unlock(&mutexMemoria);
	
	*desplazamiento += config_valores.tamanio_pagina;
}

void* copiarPagina(t_pagina* pagina, int memoria){

    void* algo = malloc(config_valores.tamanio_pagina);

    // ME TENGO QUE FIJAR SI LO QUE TENGO QUE COPIAR ESTA EN MV O MP
    if(memoria == MEM_PPAL){ 
        
        int base = pagina->frame_ppal * config_valores.tamanio_pagina;
        pthread_mutex_lock(&mutexMemoria);
        memcpy(algo, memoriaPrincipal + base, config_valores.tamanio_pagina);
		pthread_mutex_unlock(&mutexMemoria);

    }else if(memoria ==MEM_VIRT){
        int base = pagina->frame_virtual * config_valores.tamanio_pagina;

		pthread_mutex_lock(&mutexMV);
		read_mmap(base, algo);
		pthread_mutex_unlock(&mutexMV);
    }
    return algo;
}

t_tcb* sacarTripuPaginacion(int posTripuBuscado, void* losTripus){

    int desplazamiento = posTripuBuscado * 21; 

    t_tcb* tripulante = malloc(sizeof(t_tcb));

    memcpy(tripulante, losTripus + desplazamiento, 21); //COPIO TODO LO QUE HAY EN LA MEMORIA DEL TRIPULANTE BUSCADO

    return tripulante;
}

char* sacarTareasPaginacion(void* infoProceso , int size ){
    char* tareas = malloc(size);
    
    memcpy(tareas, infoProceso, size);
    return tareas;
}

//GUARDAR EN MV
void guardarMemoriaVirtual(t_pagina* pagina){
	
    void* algo = copiarPagina(pagina, MEM_PPAL);

	//VEO SI ESTA EN EL DISCO
	if(pagina->frame_virtual == -1){           //Si es -1 no esta en disco
		
		//LA GUARDO EN EL PRIMER FRAME LIBRE
		t_list* framesLibres = buscarFramesSinOcupar(MEM_VIRT);//FALTA ESTA FUNCION O ecplise no la encuentra
		t_frame* frameEscogido = list_get(framesLibres,0);
		
		log_info(logger, "Guardo la pagina %d en el frame %d de la memoria virtual", pagina->id, frameEscogido->id);
		
		//COPIO LA PAGINA EN DISCO
		ocuparMemoriaPaginacion(algo, config_valores.tamanio_pagina, frameEscogido->id, 0, MEM_VIRT);
		
		//OCUPO EL BITMAP EN MV
		ocuparFrame(frameEscogido, MEM_VIRT);
		
		//DESOCUPO EL BITMAP EN MP
		desocuparFrame(pagina->frame_ppal, MEM_PPAL); // FALTA EN .h
		
		//PONGO EL NUMERO DE FRAME
		pagina->frame_virtual = frameEscogido->id;
	
		eliminarLista(framesLibres);
	}
	else{                                   //ACTUALIZO LA PAGINA	
		desocuparFrame(pagina->frame_ppal, MEM_PPAL);//DESOCUPO EL FRAME EN MP
		ocuparMemoriaPaginacion(algo, config_valores.tamanio_pagina, pagina->frame_virtual, 0, MEM_VIRT); //Ocupo el mismo lugar que lo anterior con lo nuevo
		log_info(logger, "Actualice la pagina %d en el frame %d de mem virtual", pagina->id, pagina->frame_virtual);
	}
	
	//CAMBIO LOS BITS
	pagina->presencia = 0; //YA NO ESTA EN PPAL
	pagina->modificado = 0;//SE GUARDO EN DISCO
	cleanLock(pagina);//LA DESLOCKEO A LA PAGINA
	
	free(algo);
}

//BUSCAR


t_list* buscarPaginas(int nPrimerPagina, int size, t_tabla_pagina* unaTabla, int relleno){
	
	t_list* paginasOcupadas = list_create();
	t_pagina* primerPagina = list_get(unaTabla->paginas, nPrimerPagina);
	pthread_mutex_lock(&mutexPags);
	primerPagina->uso = 1; //COMO ACCEDI A LA PAG -> MODIFICO EL USO
	primerPagina->tiempo_uso = obtenerTiempo(); //COMO ACCEDI -> ACTUALIZO EL TIEMPO DE USO
	pthread_mutex_unlock(&mutexPags);
	//SI EL TAMANIO DEL ELEMENTO ES MENOR O IGUAL A LO QUE ESTA OCUPADO EN LA PAGINA
	if(size <= relleno){   //relleno seria lo que le falta para completar la pagina, si lo que quiero es menor a este, solo uso una pag
		list_add(paginasOcupadas, primerPagina);
		
	}else{	//SI NO -> EL RESTO VA A ESTAR EN LA PAGINA QUE LE SIGUE 
		int nPagSig = nPrimerPagina + 1;
		
		t_list* otrasPag = buscarPaginas(nPagSig, size - relleno, unaTabla, config_valores.tamanio_pagina);
		
		//AGREGAMOS LAS PAGINAS OCUPADAS 
		list_add(paginasOcupadas, primerPagina); 
		agregarPaginasALista(paginasOcupadas, otrasPag);
		
		list_destroy(otrasPag);
	}
	return paginasOcupadas;
}

void agregarPaginasALista(t_list* lista, t_list* paginas){
	
	list_add_all(lista, paginas);
}

char* buscarTareaPaginacion(int idPatota, int proxInst, int* esUltima){

    char* tareas = buscarTareasPaginacion(idPatota);
	char* tarea = separarTareas(tareas, proxInst, esUltima); //ME DEVUELVE LA TAREA Y DICE SI ES LA ULTIMA

	return tarea;
}

char* buscarTareasPaginacion(int idPatota){

    t_tabla_pagina* unaTabla = buscarTablaPagina(idPatota);
    char* tareas;

    //HAY QUE SACAR LA INFORMACION DEL PROCESO QUE ESTA EN LAS PAGINAS DE LA TABLA BUSCADA
    t_list* paginas = buscarPaginas(0, unaTabla->tamanioTareas, unaTabla, config_valores.tamanio_pagina);
    pthread_mutex_lock(&mutexCopiarPags);
    void* infoProceso = copiarPaginasMP(paginas);
    pthread_mutex_unlock(&mutexCopiarPags);
    tareas = sacarTareasPaginacion(infoProceso, unaTabla->tamanioTareas);
  
    free(infoProceso);
	list_destroy(paginas);
	
    return tareas;
}


t_tcb* buscarTripulantePaginacion(int idTripu, int idPatota){

	t_tcb* unaTcb;
   	t_tabla_pagina* unaTabla = buscarTablaPagina(idPatota);
    //Busco la primer pagina de la tcb que busco
	
	int nTripu = posTripuLista(idTripu, unaTabla->idTripus);
	
	int base = unaTabla->tamanioTareas + 8 + 21 * nTripu; 
	
	int nPrimerPagina = base / config_valores.tamanio_pagina; //Con esto tengo en que pagina empieza
	
	int sobrante = ((nPrimerPagina+1) * config_valores.tamanio_pagina) - base;
	
	t_list* paginas = buscarPaginas(nPrimerPagina, 21, unaTabla, sobrante);
	
	//Copio las paginas a un void * (tambien me fijo si estan o no en mp)
	pthread_mutex_lock(&mutexCopiarPags);
	void* choclo = copiarPaginasMP(paginas);
	pthread_mutex_unlock(&mutexCopiarPags);
	//Ahora necesito buscar donde esta lo que quiero
	
	if((base % config_valores.tamanio_pagina) == 0) //Si el resto es 0, estoy en el inicio de una pagina
	{
		unaTcb = copiarTcbPag(choclo, 0);		
	}
	else//aca tengo la tcb desplazada dentro de la pag
	{	
		int desplazamiento = base % config_valores.tamanio_pagina;
		unaTcb = copiarTcbPag(choclo, desplazamiento);
	}
	
	list_iterate(paginas,(void*)cleanLock);//Libero las paginas

	free(choclo);
	list_destroy(paginas);
	
	return unaTcb;
}

t_tcb* copiarTcbPag(void* chocloDeTripus, int desplazamiento){

    t_tcb* unaTcb = malloc(sizeof(t_tcb));

    memcpy(unaTcb, chocloDeTripus + desplazamiento, 21);

    return unaTcb;
}

int buscarPosicionTripuEnLista(int idTripu, t_list* idTripus){
    
    int posicion=0;

    for(int i=0; i < list_size(idTripus); i++){
        
        int* idTripuActual = list_get(idTripus, i); //AGARRO EL ID EN LA POS i
        
        if(*idTripuActual == idTripu){ //SI ES, DEJO DE BUSCAR
            break;
        }

        //SI NO -> ME FIJO EN LA QUE SIGUE
        posicion++;
    }

    return posicion;
}

t_tabla_pagina* buscarTablaPagina(int pid){
	
	t_tabla_pagina* unaTabla;
	
	int mismoId(t_tabla_pagina* tabla)
	{
		return (tabla->idPatota == pid);
	}
	
    pthread_mutex_lock(&mutexListaTablas);
	unaTabla = list_find(tablaDePaginas, (void*)mismoId);
	pthread_mutex_unlock(&mutexListaTablas);

	return unaTabla;	
}

t_tabla_pagina* buscarTablaConTidPag(int tid){
	
	
	int tieneUnTripulante(t_tabla_pagina* unaTabla)
	{
		return (tieneTripu(unaTabla,tid));
	}
	
	
	pthread_mutex_lock(&mutexListaTablas);
	t_tabla_pagina* unaTabla = list_find(tablaDePaginas, (void*) tieneUnTripulante);	
	pthread_mutex_unlock(&mutexListaTablas);
	
	return unaTabla;
	
}

int buscarDirPatotaPag(int idPato){//TODO ESTO PODRIA ESTAR MAL
    t_tabla_pagina* unaTabla;

    unaTabla = buscarTablaPagina(idPato);

    return unaTabla->direPcb;
}

t_list* buscarFramesSinOcupar(int mem){
    t_list* framesLibres = list_create();
    int base = 0;

    if(mem == MEM_PPAL){
        pthread_mutex_lock(&mutexBitmapMP);
        while(base < cant_frames_ppal){
            if(bitarray_test_bit(frames_ocupados_ppal, base) == 0){ //VE LOS 0 EN EL BITMAP
				t_frame* unFrame = malloc(sizeof(t_frame));//Va aca adentro porque pertence al loop
                unFrame->id = base;
                list_add(framesLibres, unFrame);
            }
        base++;
    }   
	pthread_mutex_unlock(&mutexBitmapMP);	
    }else if(mem == MEM_VIRT){
		pthread_mutex_lock(&mutexBitmapMV);
        while(base < (cant_frames_virtual)){
			
				if(bitarray_test_bit(frames_ocupados_virtual,base) == 0){//si el bitmap dice 0, el frame esta vacio
					t_frame* unFrame = malloc(sizeof(t_frame));//Va aca adentro porque pertence al loop
					unFrame->id = base;
					list_add(framesLibres, unFrame);
				}
				base ++;
			}
	pthread_mutex_unlock(&mutexBitmapMV);
    }
    return framesLibres; 
}

t_list* buscarPaginasDisponiblesEn(t_tabla_pagina* tabla){

    t_list* paginasDisponibles;
    t_list* paginasDeLaTabla = tabla->paginas;

    int hayLugar(t_pagina* pagina){
        return (pagina->tamanioDisponible > 0);
    } 

    paginasDisponibles = list_filter(paginasDeLaTabla, (void*)hayLugar);

    return paginasDisponibles;
}

int hayLugarEnLasPaginas(t_tabla_pagina* tabla){

    t_list* paginas= buscarPaginasDisponiblesEn(tabla);

    if(list_is_empty(paginas)){//NO HAY PAGINAS CON ESPACIO DISPONIBLE 
        list_destroy(paginas);
        return 0;
    }
    else 
    {
        list_destroy(paginas);
        return 1;    	
    }
}

t_tabla_pagina* buscarTablaPagConTid(int idTripu){

    int tieneUnTripulante(t_tabla_pagina* unaTabla){
        return(tieneTripu(unaTabla, idTripu));
    }

    pthread_mutex_lock(&mutexListaTablas);
    t_tabla_pagina* unaTabla = list_find(tablaDePaginas, (void*)tieneUnTripulante);
    pthread_mutex_unlock(&mutexListaTablas);
    
    return unaTabla;
}

int tieneTripu(t_tabla_pagina* unaTabla, int idTripu){

    int mismoId(int* unId){
        return *unId == idTripu;
    }

    if(list_find(unaTabla->idTripus, (void*)mismoId) != NULL){
        return 1;
    }else{
        return 0;
    }
}

t_tcb*  p_buscarTripulante(int idTripu){
    
    t_tabla_pagina* tabla = buscarTablaPagConTid(idTripu);

    int idPatota = tabla->idPatota;
    

    
    t_tcb* tripu = buscarTripulantePaginacion(idTripu, idPatota);
    


    return tripu;
}


t_list* buscarPaginasMP(){
	
	t_list* paginas = list_create();
	
	//BUSCO TODAS LAS PAGINAS
	
	void paginasTabla(t_tabla_pagina* unaTabla)
	{	
		list_add_all(paginas, unaTabla->paginas);		
	}
	pthread_mutex_lock(&mutexListaTablas);
	list_iterate(tablaDePaginas, (void*)paginasTabla);
	pthread_mutex_unlock(&mutexListaTablas);
	//FILTRO LAS QUE TENGAN EL BIT DE PRESENCIA EN 1 => ESTAN EN MP
	
	int presente(t_pagina* unaPag)
	{
		return (unaPag->presencia == 1);
	}
	
	t_list* paginasEnMp = list_filter(paginas, (void*)presente);
	
	list_destroy(paginas);
	return paginasEnMp;
}

int memoriaDisponiblePag(int mem){
	
	int espaciosLibres = 0;
	int desplazamiento = 0;
	
	if(mem == MEM_PPAL){
	
		while(desplazamiento < cant_frames_ppal){
			
			pthread_mutex_lock(&mutexBitmapMP);
			if(bitarray_test_bit(frames_ocupados_ppal,desplazamiento) == 0){
				
				espaciosLibres++; 
			}
			pthread_mutex_unlock(&mutexBitmapMP);

			desplazamiento++;
	
		}
	}else if(mem == MEM_VIRT){
	
		while(desplazamiento < cant_frames_virtual){
			
			pthread_mutex_lock(&mutexBitmapMV);
			if(bitarray_test_bit(frames_ocupados_virtual, desplazamiento) == 0){
				
				espaciosLibres++;
			}
			pthread_mutex_unlock(&mutexBitmapMV);
			desplazamiento++;	
		}
	}
	
	return espaciosLibres;	
}

//CREAR
t_tabla_pagina* crearTablaDePaginas(int idPatota, int size, int cantTripus){
	
	t_tabla_pagina* unaTabla = malloc(sizeof(t_tabla_pagina));
	
	unaTabla->idPatota = idPatota;
	unaTabla->tamanioTareas = size;
    unaTabla->cantTripus = cantTripus;
    //unaTabla-> direPcb= direPcb;
	unaTabla->paginas = list_create();
	unaTabla->idTripus = list_create();
	
	pthread_mutex_lock(&mutexListaTablas);
	list_add(tablaDePaginas, unaTabla);
	pthread_mutex_unlock(&mutexListaTablas);
	
	return unaTabla;
}

t_pagina* crearPagina(t_frame* frame, int espacioDisp){

    t_pagina* unaPagina = malloc(sizeof(t_pagina));

    unaPagina-> id = generarId(); 
	unaPagina->frame_ppal = frame->id;
	unaPagina->frame_virtual = -1; //Lo pongo en -1 como para decir que no esta en mem virtual
	unaPagina->tamanioDisponible = espacioDisp;
	unaPagina->fragInterna = 0;
	unaPagina->presencia = 1;//esta en mp
	unaPagina->modificado = 1;//Lo pongo en 1 asi se carga en disco
	unaPagina->lock = 0;
	unaPagina->uso = 0;
	unaPagina->tiempo_uso = obtenerTiempo();

    return unaPagina;
}



int obtenerTiempo(){
	pthread_mutex_lock(&mutexTiempo);
	double t = tiempo;
	tiempo++;
	pthread_mutex_unlock(&mutexTiempo);
	return t;
}

//ACTUALIZAR
void p_actualizarTripulante(t_tcb* nuevaTcb){
	//BUSCO LA TABLA DONDE ESTA EL TRIPULANTE (TIENE LA LISTA DE IDS)
	t_tabla_pagina* unaTabla = buscarTablaPagConTid(nuevaTcb->idTripulante);
	

	pthread_mutex_lock(&mutexActualizar);
	sobreEscribirTcbPag(nuevaTcb,unaTabla);
	pthread_mutex_unlock(&mutexActualizar);


}

void sobreEscribirTcbPag(t_tcb* tcb, t_tabla_pagina* unaTabla){
	
	//CALCULO EN QUE FRAME EMPIEZA EL TRIPU CON LA POSICION EN LA LISTA
	int nTripu = posTripuLista(tcb->idTripulante, unaTabla->idTripus);
	
	//PRIMERO TENGO LAS TAREAS, DESPUES LA PATOTA Y AVANZO CON EL NTRIPU HASTA LLEGAR A LA POS DONDE ESTA GUARDADO
	int base = unaTabla->tamanioTareas + 8 + 21 * nTripu; 
	
	//BUSCO LA PAGINA DONDE ESTA GUARDADO
	int nPrimerPagina = base / config_valores.tamanio_pagina; //CON ESTO TENGO LA PRIMERA PAGINA DONDE EMPIEZA EL TRIPU
	
	//SOBRANTE ES EL CACHITO QUE OCUPA LA TCB EN ESA PAGINA
	int relleno = ((nPrimerPagina+1) * config_valores.tamanio_pagina) - base;
	
	//BUSCO LA O LAS PAGINAS QUE OCUPA LA TCB
	t_list* paginas = buscarPaginas(nPrimerPagina, 21, unaTabla, relleno); //Le tengo que pasar cuanto le queda a la pagina actual
	
	if(list_all_satisfy(paginas, (void*)enMP)){ 
		
		//BUSCO LOS FRAMES QUE OCUPAN ESAS PAGINAS
		t_list* frames = buscarFramesDePaginas(paginas);
		
		//VEO SI EMPIEZA EN EL MEDIO O DESDE EL INICIO DE UN FRAME Y ACTUALIZO
		int desplazamiento;
		if((base % config_valores.tamanio_pagina) == 0){			//Si el resto es 0, estoy en el inicio de una pagina
			desplazamiento=  0;	
		}else{														//aca tengo la tcb desplazada dentro de la pag
			desplazamiento = base % config_valores.tamanio_pagina;	//EL RESTO ME DA EL CORRIMIENTO DEL PTR DESDE EL INICIO DE LA PAGINA
		}
		actualizarElementoPag(tcb, frames, 21, desplazamiento);
		
		//CAMBIO EL BIT DE MODIFICADO DE CADA UNA DE LAS PAGINAS
		list_iterate(paginas, (void*)setModificado); 
		eliminarLista(frames);
	}else{	
		
		//LOCKEO LAS PAGINAS QUE NECESITO
		list_iterate(paginas,(void*)setLock);
		
		//FILTRO LAS QUE ESTAN EN MV Y LAS REVIVO
		t_list* paginasEnMV = list_filter(paginas, (void*)enMV);
		revivirPaginas(paginasEnMV);
		
		//DEBERIAN ESTAR TODAS EN MP, VUELVO A EJECUTAR ESTA FUNCION
		sobreEscribirTcbPag(tcb, unaTabla);
		
		//LIBERO LAS PAGINAS
		list_iterate(paginas,(void*)cleanLock);
	}
	list_destroy(paginas);
}

int posTripuLista(int tid,t_list* tripIds){ //BUSCA LA POSICION DE MI TCB
	
	int contador = 0;
	pthread_mutex_lock(&mutexCopiarPags);
	for(int i = 0; i < list_size(tripIds) ; i++){
		
		int* id = list_get(tripIds,i);
		
		if(*id == tid)break;
		else contador++;
	}
	pthread_mutex_unlock(&mutexCopiarPags);
	return contador;
}

t_list* buscarFramesDePaginas(t_list* paginas){
	
	t_list* frames;
	
	t_frame* pagToFrame(t_pagina* unaPag){	

		t_frame* frame = malloc(sizeof(t_frame));
		frame->id = unaPag->frame_ppal;
		
		return frame;
	}
	
	frames = list_map(paginas, (void*) pagToFrame);
	
	return frames;
}

void actualizarElementoPag(void* algo, t_list* frames,  int size, int desplazamiento){

	if(list_size(frames) == 1){ //USO UNA SOLA PAGINA
		t_frame* primerFrame = list_get(frames, 0);
		ocuparMemoriaPaginacion(algo, size, primerFrame->id, desplazamiento, MEM_PPAL);//LO GUARDO EN EL FRAME
	}else{ 						//USO MAS DE UNA PAGINA
		int relleno = config_valores.tamanio_pagina - desplazamiento; //RELLENO ES LO Q YA GUARDE
		
		//TOMA UNO DE LA LISTA (EL PRIMERO) Y LO HACE UNA LISTA PARA PODER HACER RECURSION
		t_list* unFrame = list_take(frames,1);
		
		actualizarElementoPag(algo, unFrame, relleno, desplazamiento);//Guardo una parte en un frame, me falta guardar relleno
		
		//SACO EL PRIMER FRAME DE LA LISTA
		list_remove(frames, 0); 
		
		actualizarElementoPag(algo + relleno, frames, size - relleno, 0); //PUEDE SER UNA O MAS PAGINAS
		eliminarLista(unFrame);
		
		//eliminarLista(frames);
	}
	//list_destroy(framesLibres); o eliminarLista(framesLibres);
}

//EXPULSAR
void p_expulsarTripulante(int idTripu){
	
	pthread_mutex_lock(&mutexEliminar);
	
	t_tabla_pagina* unaTabla = buscarTablaPagConTid(idTripu);
	
	//CALCULO EN QUE FRAME EMPIEZA EL TRIPU
	int nTripu = posTripuLista(idTripu, unaTabla->idTripus);
	

	
	//CALCULO LA BASE DONDE EMPIEZA EL TRIPU
	int base = unaTabla->tamanioTareas + 8 + 21 * nTripu; 
	
	int nPrimerPagina = base / config_valores.tamanio_pagina; //EN QUE PAGINA EMPIEZA
	
	int relleno = ((nPrimerPagina+1) * config_valores.tamanio_pagina) - base;	//RELLENO ES LO QUE OCUPA EL TRIPU EN LA PRIMERA PAGINA
	
	//BUSCO LA O LAS PAGINAS QUE OCUPA LA TCB
	t_list* paginas = buscarPaginas(nPrimerPagina, 21, unaTabla, relleno); //LE PASO CUANTO LE QUEDA A LA PAGINA ACTUAL
	
	//LIBERO LAS PAGINAS NECESARIAS
	liberarPaginas(paginas, 21, unaTabla, base, relleno);	

	int* cancel = malloc(sizeof(int));
	*cancel = -1;
	list_replace_and_destroy_element(unaTabla->idTripus,nTripu, cancel, (void*)eliminarAlgo);//Si no se muestra se pone en -1
	
	log_info(logger,"Se expulso el tripulante %d", idTripu);
		
	list_destroy(paginas);
	
	int todosMenosUno(int* id)
	{
		return *id == -1;
	}
	
	if(list_all_satisfy(unaTabla->idTripus, (void*)todosMenosUno)){//No hay mas tripus, borro la patota
		
		eliminarPatota(unaTabla);
		
		
	}
	pthread_mutex_unlock(&mutexEliminar);
	
}

void eliminarPatota(t_tabla_pagina* unaTabla){
	
	log_info(logger, "Se elimino la patota %d",unaTabla->idPatota);
	
	//Libero las paginas
	list_iterate(unaTabla->paginas,(void*)matarPagina);
	
	//Borro el quilombo
	
	int mismoId(t_tabla_pagina* tabla)
	{
		return unaTabla->idPatota == tabla->idPatota; 
	}
	pthread_mutex_lock(&mutexListaTablas);
	list_remove_and_destroy_by_condition(tablaDePaginas, (void*) mismoId, (void*) eliminarTablaPaginas);
	pthread_mutex_unlock(&mutexListaTablas);
}

void matarPagina(t_pagina* unaPagina){
	
	//LA SACO DEL BITMAP
	desocuparFrame(unaPagina->frame_ppal, MEM_PPAL);
	
	//VEO SI ESTA EN MV
	if(unaPagina->frame_virtual != -1){//SI ES -1 NO ESTA
		
		//DESOCUPO EL FRAME EN MV 
		desocuparFrame(unaPagina->frame_virtual, MEM_VIRT);
	}
	

	
	
}


void liberarPaginas(t_list* paginas, int size, t_tabla_pagina* unaTabla, int base, int relleno){

	if(list_size(paginas) > 1){ //OCUPO MAS DE UNA PAGINA
			
		if((base % config_valores.tamanio_pagina) == 0){	//SI ESTOY AL PRINCIPIO DE UN FRAME, LO LIBERO ENTERO
			liberarPagina(list_get(paginas,0), unaTabla);	//Como tengo mas de una pagina, y empiezo desde el inicio de la misma, 100% seguros que ocupa toda la misma
			list_remove(paginas,0);
			liberarPaginas(paginas, size - config_valores.tamanio_pagina, unaTabla, base + config_valores.tamanio_pagina,config_valores.tamanio_pagina);
		}else{ 												//SI HAY OTRAS COSAS EN LA PAGINA
			t_pagina* primerPag = list_get(paginas,0);
			primerPag->fragInterna += relleno; 		//el cachito que ocupaba de la pagina queda como frag interna
			if(primerPag->fragInterna == config_valores.tamanio_pagina){//Si el espacio libre mas lo que tengo mas la frag interna es un frame, NO HAY NADA MAS EN LA PAG Y LA PUEO LIBERAR
				liberarPagina(primerPag, unaTabla);					
			}
			list_remove(paginas,0);
			liberarPaginas(paginas, size - relleno, unaTabla, base + relleno, config_valores.tamanio_pagina);//Muevo la base el sobramte, que es el cachito que ocupa
		}
	}else{ 													//Aca tengo que ver si ocupa toda la pagina o solo un cachito, pero nunca se sale de la misma
		t_pagina* unicaPag = list_get(paginas,0);
		
		if(unicaPag->fragInterna + unicaPag->tamanioDisponible + size == config_valores.tamanio_pagina) {//SI LA SUMA DE TODO ESO DA IGUAL AL TAM DE UNA PAGINA, ENTONCES SIGNIFICA QUE NO HAY NADA MAS Y LA PUEDO BORRAR
			liberarPagina(unicaPag, unaTabla);		//LIBERO LA PAGINA			
		}
		else unicaPag->fragInterna += size; //LO Q "BORRE" (EL SIZE) QUEDA COMO FRA INTERNA 
		//En el resto de los casos no me importa liberar la pagina, porque la ocupa otra cosa
	}
}

void liberarPagina(t_pagina* unaPagina, t_tabla_pagina* unaTabla){
	
	//ANULO LA PAGINA
	unaPagina->id = -1;//Asi no la encuentra nadie
	
	//LA SACO DEL BITMAP
	desocuparFrame(unaPagina->frame_ppal, MEM_PPAL);
	
	//ANULO EL FRAME DE LA PAGINA 
	unaPagina->frame_ppal = -1;
	
	//VEO SI ESTA EN MV
	if(unaPagina->frame_virtual != -1){//SI ES -1 NO ESTA
		
		//DESOCUPO EL FRAME EN MV 
		desocuparFrame(unaPagina->frame_virtual, MEM_VIRT);

		//ANULO LA PAGINA
		unaPagina->frame_virtual = -1;
	}
	
	unaPagina->tamanioDisponible = 0;
}

void liberarMemoriaPaginacion(){
	
	bitarray_destroy(frames_ocupados_ppal);
	bitarray_destroy(frames_ocupados_virtual);
	free(data);
	free(data2);
	pthread_mutex_lock(&mutexListaTablas);
	list_destroy_and_destroy_elements(tablaDePaginas, (void*)eliminarTablaPaginas);
	pthread_mutex_unlock(&mutexListaTablas);
}

void eliminarTablaPaginas(t_tabla_pagina* unaTabla){
	
	eliminarLista(unaTabla->paginas);
	eliminarLista(unaTabla->idTripus);
	free(unaTabla);
}


//SWAP
void swap(int cantPag){ 
	log_info(logger,"Swapeando ando");
	for(int i = 0; i < cantPag; i++){
		
		reemplazarPagina();	//REEMPLAZA LA CANTIDAD DE PAGINAS QUE LE MANDO
	}
}

void reemplazarPagina(){
	
	//BUSCO TODAS LAS PAGINAS QUE SE PUEDAN REEMPLAZAR
	t_list* paginasEnMp = buscarPaginasMP(); 
	
	int noLock(t_pagina* pag){
        
		return (pag->lock == 0);
	}
	
	t_list* paginasSinLock = list_filter(paginasEnMp, (void*)noLock);//FILTRO LAS QUE ESTAN LOCKEADAS -> ESTAS LAS NECESITO //TODO QUE PASA SI ESTAN TODAS LOCKEADAS???
	
	if(list_is_empty(paginasSinLock)){
		log_error(logger, "No hay paginas para sacar de la memoria ppal");
	}
	
	if(string_equals_ignore_case(config_valores.algoritmo_reemplazo, "LRU")){
		reemplazarConLRU(paginasSinLock);
	}
	else if(string_equals_ignore_case(config_valores.algoritmo_reemplazo, "CLOCK")){
		reemplazarConClock(paginasSinLock);
	}
	
	list_destroy(paginasEnMp);
	list_destroy(paginasSinLock);	
}

//LRU
void reemplazarConLRU(t_list* paginasEnMp){
	
	int masVieja(t_pagina* unaPag, t_pagina* otraPag){
		
		return (otraPag->tiempo_uso > unaPag->tiempo_uso); //LA QUE ESTA HACE MAS TIEMPO
	}
	
	list_sort(paginasEnMp, (void*) masVieja); //FILTRO Y PONGO A LA MAS VIEJA PRIMERO -> ORDENA DE MAS VIEJA A MAS NIEVA
	
	//COMO REEMPLAZO SEGUN LRU, ELIJO LA PRIMERA QUE ES LA MAS VIEJA
	t_pagina* paginaReemplazo = list_get(paginasEnMp, 0);
	log_info(logger, "Voy a reemplazar la pagina %d que estaba en el frame %d", paginaReemplazo->id, paginaReemplazo->frame_ppal);
	setLock(paginaReemplazo);
	
	//SI EL BIT DE MODIFICADO ES 1, LA GUARDO EM MV -> PORQUE TIENE CONTENIDO DIFERENTE A LO QUE ESTA EN MV
	if(paginaReemplazo->modificado == 1)
	{
		guardarMemoriaVirtual(paginaReemplazo);
	}
	else
	{
		//EN LA MP, DESOCUPO EL FRAME EN EL BITMAP
		desocuparFrame(paginaReemplazo->frame_ppal, MEM_PPAL);
        //AHORA ESTA PAGINA YA NO ESTA EN MP, CAMBIO EL BIT DE PRESENCIA
		paginaReemplazo->presencia = 0;
		cleanLock(paginaReemplazo);
	}
}

//CLOCK
void reemplazarConClock(t_list* paginasEnMp){
	
	//BUSCO LA PAGINA A REEMPLAZAR SEGUN CLOCK
	t_pagina* paginaReemplazo = elegirPaginaClock(paginasEnMp);
	log_info(logger, "Voy a reemplazar la pagina %d que estaba en el frame %d", paginaReemplazo->id, paginaReemplazo->frame_ppal);
	setLock(paginaReemplazo);
	
	if(paginaReemplazo->modificado == 1)
	{
		guardarMemoriaVirtual(paginaReemplazo);
	}
	else
	{
		//EN LA MP, DESOCUPO EL FRAME EN EL BITMAP
		desocuparFrame(paginaReemplazo->frame_ppal, MEM_PPAL);
        //AHORA ESTA PAGINA YA NO ESTA EN MP, CAMBIO EL BIT DE PRESENCIA
		paginaReemplazo->presencia = 0;
		cleanLock(paginaReemplazo);
	}
}

t_pagina* elegirPaginaClock(t_list* paginas){
	/*int i;
	if(list_get(paginas,posClock+1) == NULL){//SI EL PUNTERO APUNTA MAS ALLA DE LA CANT DE PAGINAS, EMPIEZO POR LA PRIMERA
		i = 0;
	}else i = posClock+1;//ACA EL PUNTERO ENTRA DENTRO DE LAS PAGS
	t_pagina* escogida;
	log_info(logger,"Paginas para reemplazar son: ");
	list_iterate(paginas, (void*)mostrarPagina);
	
	while(list_get(paginas, i) != NULL){
		
		t_pagina* pagina = list_get(paginas, i);
		
        //BUSCO LA PRIMERA CUYO BIT DE USO ESTE EN 0
		if(pagina->uso == 0)
		{
			escogida = pagina;
			posClock=i;
			break;
		}
		//SI NO ENTRO, TIENE EL BIT EN 1 => LO PONGO EN 0 
		pagina->uso = 0; 

		if((i+1) == list_size(paginas)){
            i = 0; //SI RECORRI TODA LA LISTA Y NO ENCONTRE NINGUNA PAG EMPIEZO OTRA VEZ 
        }else{ 
            i++;
        }
	}*/
	int pagSeleccionada;
	int noEncontre = 1;
	int i= posClock;
	t_pagina* escogida;
	log_info(logger,"Paginas para reemplazar son: ");
	list_iterate(paginas, (void*)mostrarPagina);
	while(noEncontre){
		
		if(i>= list_size(paginas)){
			i = 0;
		}
		
		t_pagina* pagina = list_get(paginas,i);
		
		if(pagina->uso == 0){
			pagSeleccionada = i;
			posClock = i;
			noEncontre=0;
		}else{
			pagina->uso = 0;
			i++;			
		}
		
		
	}
	
	escogida = list_get(paginas,pagSeleccionada);
	
	
	
	
	return escogida;
}

void revivirPaginas(t_list* paginasEnMV){
	
	list_iterate(paginasEnMV, (void*)revivirPagina);
	list_destroy(paginasEnMV);
}


void revivirPagina(t_pagina* unaPagina){
	
	//COPIO LO QUE HAY EN ESA PAGINA QUE ESTA EN DISCO
	void* algo = copiarPagina(unaPagina, MEM_VIRT);
	
	//GUARDO LO NUEVO EN MP
	t_list* lPagina = guardarElementoPaginacion(algo, config_valores.tamanio_pagina);//DEVUELVE UNA PAGINA, LA QUE GUARDASTE, SOLO NECESITO EL FRAME DONDE LO GUARDE
	t_pagina* pagina = list_get(lPagina,0); //PRIMERA Y UNICA
	
	int nuevoFrame = pagina->frame_ppal; //ES EL FRAME DONDE SE GUARDO LA PAG
	unaPagina->frame_ppal = nuevoFrame; //LO ACTUALIZO EN LA PAGINA QUE REVIVO

	log_info(logger, "Revivi la pagina %d en el frame %d", unaPagina->id, nuevoFrame);
	
	//CAMBIO EL BIT DE PRESENCIA PQ AHORA ESTA EN MP
	unaPagina->presencia = 1;
	
	//ACTUALIZO EL TIEMPO
	unaPagina->tiempo_uso = obtenerTiempo();
	
	eliminarLista(lPagina);
	free(algo);	
}

void ocuparFrame(t_frame* unFrame, int mem){
	
    if(mem == MEM_PPAL){
        pthread_mutex_lock(&mutexBitmapMP);
        bitarray_set_bit(frames_ocupados_ppal, (unFrame->id));
        pthread_mutex_unlock(&mutexBitmapMP);	
    }else if(mem == MEM_VIRT){
        pthread_mutex_lock(&mutexBitmapMV);
		bitarray_set_bit(frames_ocupados_virtual,(unFrame->id));
		pthread_mutex_unlock(&mutexBitmapMV);
    }
}

void desocuparFrame(int frame, int mem){
	
	if(mem == MEM_PPAL){
		pthread_mutex_lock(&mutexBitmapMP);
		bitarray_clean_bit(frames_ocupados_ppal, frame);
		pthread_mutex_unlock(&mutexBitmapMP);	
	}
	else if(mem == MEM_VIRT)
	{
		pthread_mutex_lock(&mutexBitmapMV);
		bitarray_clean_bit(frames_ocupados_virtual, frame);
		pthread_mutex_unlock(&mutexBitmapMV);
	}

	
}

int calcularDirLogica(t_pagina* pagina, int desplazamiento){

    int dirLogica = (pagina->id * config_valores.tamanio_pagina) + desplazamiento;
    return dirLogica; 
}

//MMAP

void write_mmap(void* algo,int size,int base){
	
	void * map;
	int fd = 0;
	struct stat fileInfo;
	size_t fileSizeNew;
	
	if ((fd = open(memoriaVirtualPath, O_RDWR | O_CREAT, (mode_t)0664 )) == -1) {
		perror("open");
		exit(1);
	}
	if (stat(memoriaVirtualPath, &fileInfo) == -1) {
		perror("stat");
		exit(1);
	}
	
	
	// Stretch the file size to write the array of char
	//fileSizeOld = fileInfo.st_size;
	
	fileSizeNew = fileInfo.st_size + size;
	
	if (ftruncate(fd, fileSizeNew) == -1) {
		close(fd);
		perror("Error resizing the file");
		exit(1);
	}
	
	
	

	// mmap to write
	map = mmap(0, fileSizeNew, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (map == MAP_FAILED) {
		close(fd);
		perror("mmap");
		exit(1);
	}
	
	
	//Copio algo en mi memoria virtual
	
	memcpy(map + base, algo, size);
	
	
	
	// Write it now to disk
	if (msync(map, fileSizeNew, MS_SYNC) == -1) {
		perror("Could not sync the file to disk");
	}
	// Free the mmapped memory
	if (munmap(map, fileSizeNew) == -1) {
		close(fd);
		perror("Error un-mmapping the file");
		exit(1);
	}
	// Un-mmaping doesn't close the file, so we still need to do that
	close(fd);
	
}

void read_mmap(int base, void* algo){
	
	void * map;
	int fd = 0;
	struct stat fileInfo;

	
	if ((fd = open(memoriaVirtualPath, O_RDWR | O_CREAT, (mode_t)0664 )) == -1) {
		perror("open");
		exit(1);
	}
	if (stat(memoriaVirtualPath, &fileInfo) == -1) {
		perror("stat");
		exit(1);
	}

	// mmap to write
	map = mmap(0, fileInfo.st_size, PROT_READ, MAP_SHARED, fd, 0);
	if (map == MAP_FAILED) {
		close(fd);
		perror("mmap");
		exit(1);
	}
	
	
	//Copio algo en mi memoria virtual
	
	memcpy(algo, map + base , config_valores.tamanio_pagina);
	
	
	// Free the mmapped memory
	if (munmap(map, fileInfo.st_size) == -1) {
		close(fd);
		perror("Error un-mmapping the file");
		exit(1);
	}
	// Un-mmaping doesn't close the file, so we still need to do that
	close(fd);

	
}

void dumpPaginacion(void){

FILE * file;

    char* nombreDeArchivo = dameNombre();

    log_info(logger,"Creo un archivo con el nombre %s\n", nombreDeArchivo);

    char* time = temporal_get_string_time("%d/%m/%y %H:%M:%S");


    file = fopen(nombreDeArchivo, "w");

    fprintf(file, "\n\n");
    fprintf(file, "- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - \n");
    fprintf(file, "Dump: %s\n\n",time);

    mostrarMarcosPaginacion(file);

    fprintf(file, "- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - \n");

    //Proceso: 1    Segmento: 1    Inicio: 0x0000    Tam: 20b

    fclose(file);

    free(nombreDeArchivo);
    free(time);
}

void mostrarMarcosPaginacion(FILE* file){ 

    for(int i = 0; i <cant_frames_ppal; i++){
        mostrarFrame(i,file);
    }
}

void mostrarFrame(int nFrame, FILE* file){

    t_pagina* paginaEnFrame = buscarPaginaConFrame(nFrame);

    //Muestro los datos que quiero

    if(paginaEnFrame != NULL){//El frame esta ocupado

        int idPatota = buscarIdPatota(paginaEnFrame);

        fprintf(file, "Marco: %d    Estado:Ocupado        Proceso: %d    Pagina: %d \n",nFrame, idPatota, paginaEnFrame->id );

        fprintf(file,"\n\n");
    }
    else{//El frame no esta ocupado

        fprintf(file, "Marco: %d    Estado:Libre                 Proceso:  -    Pagina:  - \n",nFrame);
        fprintf(file,"\n\n");
    }
}

t_pagina* buscarPaginaConFrame(int nFrame){

    t_pagina* pagina;

    t_list* paginas = buscarPaginasMP();

    int tieneFrame(t_pagina* pag){
        return (pag->frame_ppal) == nFrame;
    }

    if(list_find(paginas, (void*)tieneFrame) != NULL){

        pagina = list_find(paginas, (void*)tieneFrame);
        list_destroy(paginas);
        return pagina;
    }else{
            list_destroy(paginas);
            return NULL;
    }
}

int buscarIdPatota(t_pagina* pagina){

    int tienePagina(t_tabla_pagina* tabla)
    {
        int mismoID(t_pagina* pag){
            return pag->id == pagina->id;
        }
        return list_any_satisfy(tabla->paginas, (void*)mismoID);
    }
    pthread_mutex_lock(&mutexListaTablas);
    t_tabla_pagina* tabla = list_find(tablaDePaginas, (void*)tienePagina);
    pthread_mutex_unlock(&mutexListaTablas);
    return tabla->idPatota;
}

void mostrarListaTripulantesPag(t_list* idLista){
	
	void mostrar(int* id)
	{	
		pthread_mutex_lock(&mutexMostrar);
		mostrarTripulantePag(*id);
		pthread_mutex_unlock(&mutexMostrar);
		
	}
	
	list_iterate(idLista, (void*)mostrar);	
}

void mostrarTripulantePag(int tid){
	
	if(tid != -1){
		t_tabla_pagina* unaTabla = buscarTablaPagConTid(tid);
		t_tcb* unaTcb = buscarTripulantePaginacion(tid, unaTabla->idPatota);//Dos accesos a la tabla por no buscar la tabla con el tid!!!!!!!!!
		
		mostrarTcb(unaTcb);
		
		free(unaTcb);
	}	
}

void iniciarSwap(){
	
	if(!existe_archivo(memoriaVirtualPath))//Si no existe el archivo, lo creo
	{
		int file = open(memoriaVirtualPath, O_WRONLY);
		ftruncate(file, config_valores.tamanio_swap);
		close(file);
	}
}

int existeArchivo(char* path)
{
	int ret = 0;
	int f = open(path,O_RDONLY);
	if (f != -1)//Si no es -1, se creo
	{
		ret = 1;
		close(f);
	}
	return ret;
}

void mostrarPagina(t_pagina* pagina){
	
	
	log_info(logger,"Pagina %d FrameMP: %d FrameMV: %d P: %d U: %d M: %d L: %d T: %d",pagina->id, pagina->frame_ppal, pagina->frame_virtual, pagina->presencia, pagina->uso, pagina->modificado, pagina->lock, pagina->tiempo_uso);
	
	
	
}

