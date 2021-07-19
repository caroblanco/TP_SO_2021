#include "mSegmentacion.h"

////////////////////////////////////////////////////////////////SEGMENTACION///////////////////////////////////////////////////////
int iniciarSegmentacion(void){
    memoriaPrincipal = malloc(config_valores.tamanio_memoria);

    if(memoriaPrincipal == NULL){
        //NO SE RESERVO LA MEMORIA
        return 0;
    }

    //LISTAS
    tablaDeSegmentosDePatotas = list_create();
    tablaDeSegmentosDeTripulantes = list_create();

    //BITARRAY
    datos = asignarMemoriaBytes(config_valores.tamanio_memoria); //LLENA LOS CAMPOS EN 0

    if(datos == NULL){
        //NO SE RESERVO LA MEMORIA
        return 0;
    }

    int tamanio = bitsToBytes(config_valores.tamanio_memoria);

    bitMapSegment = bitarray_create_with_mode(datos,tamanio, MSB_FIRST);

    return 1; //SI FALLA DEVUELVE 0
}

int puedoGuardar(int quieroGuardar){ //RECIBE CANT BYTES QUE QUIERO GUARDAR

    int tamanioLibre = tamanioTotalDisponible();
    log_info(logger, "Hay %d espacio libre, quiero guardar %d", tamanioLibre, quieroGuardar);
    if(quieroGuardar <= tamanioLibre){
        return 1;
    }else return 0; //DEVUELVE 1 SI HAY ESPACIO SUFICIENTE PARA GUARDAR LO QUE QUIERO GUARDAR

    
}


int tamanioTotalDisponible(void){
    
    int contador = 0;
    int desplazamiento = 0 ;

    while (desplazamiento < config_valores.tamanio_memoria){

    	pthread_mutex_lock(&mutexBitMapSegment);
        if((bitarray_test_bit(bitMapSegment, desplazamiento) == 0)){
            contador ++;
        }
        pthread_mutex_unlock(&mutexBitMapSegment);
        desplazamiento ++; 
    }

    return contador;
}

//GUARDAR EN MEMORIA SEGMENTOS

t_segmento* guardarTareasSeg(char* tareas){
    t_segmento* segmento;

    int tamanioTareas= strlen(tareas) + 1;
    
    segmento = guardarElemento(tareas, tamanioTareas);
    
    log_info(logger, "Guarde las tareas %s en el segmento %d",tareas,segmento->id);

    return segmento;
}

t_segmento* guardarElemento(void* elemento, int size){
    
    t_segmento* unSegmento = malloc(sizeof(t_segmento));
    t_segmento* aux;

    aux = buscarSegmentoSegunTamanio(size); // BUSCA UN SEGMENTO LIBRE PARA GUARDAR LAS TAREAS

    guardarEnMemoria(elemento, aux, size);
    
    unSegmento->id = aux->id;
    unSegmento->base = aux->base;
    unSegmento->limite= size;

    free(aux);

    return unSegmento; //DEVUELVE EL SEGMENTO QUE FUE GUARDADO
}

void guardarEnMemoria(void* elemento, t_segmento* segmento, int size){
    
    ocuparBitMap(bitMapSegment, segmento->base,size);
    ocuparMemoria(elemento, segmento->base, size);
}

void ocuparMemoria(void* tareas, int base, int size){
	pthread_mutex_lock(&mutexMemoria);
    memcpy(memoriaPrincipal+base, tareas, size);
    pthread_mutex_unlock(&mutexMemoria);
}

t_segmento* guardarPcbSeg(t_pcb* pcb){
    t_segmento* unSegmento;

    unSegmento = guardarElemento(pcb, 8);
    
    log_info(logger, "Guarde la pcb de la patota %d en el segmento %d",pcb->idPatota, unSegmento->id);

    return unSegmento;
}

t_segmento* guardarTcbSeg(t_tcb* tcb){
    t_segmento* unSegmento;

    unSegmento = guardarElemento(tcb, 21);
    
    log_info(logger, "Guarde la tcb del tripulante %d en el segmento %d",tcb->idTripulante, unSegmento->id);

    return unSegmento;
}

void eliminarTcbSeg(int idTripu){
    
    t_segmento* tripuSeg = buscarSegmentoTripulante(idTripu);

    liberarBitMap(bitMapSegment, tripuSeg->base,tripuSeg->limite);
}

void* copiarSegmentacion(t_segmento* unSegmento){
	
	void* algo;

    if(unSegmento->limite == 21){       //TRIPU
        algo = malloc(sizeof(t_tcb));
    }else if(unSegmento->limite == 8){  //PATOTA
        algo = malloc(sizeof(t_pcb));
    }else{                              //TAREAS
        algo = malloc(unSegmento->limite);
    }

	pthread_mutex_lock(&mutexMemoria);
	memcpy(algo,memoriaPrincipal + unSegmento->base, unSegmento->limite); //COPIA EN ALGO DESDE HASTA TAL LUGAR
	pthread_mutex_unlock(&mutexMemoria);
    
    return algo;
}

//TABLAS
t_tabla_patota* crearTablaPatota(int idPatota, t_segmento* pcb_seg, t_segmento* segmentoTareas){
    t_tabla_patota* tablaPatota = malloc(sizeof(t_tabla_patota));
    tablaPatota->idPatota = idPatota;
    tablaPatota->pcb = pcb_seg;
    tablaPatota->tareas = segmentoTareas;
    tablaPatota->tripusVivos = 0;

    return tablaPatota;
}

t_tabla_tripulante* crearTablaTripulante(int idTripulante, t_segmento* tcb_seg){
    t_tabla_tripulante* unaTabla = malloc(sizeof(t_tabla_tripulante));
	
	unaTabla->idTripulante = idTripulante;
	unaTabla->seg_tcb = tcb_seg;
	
	return unaTabla;	
}

//BUSCAR

t_tabla_tripulante* buscarTablaTripulante(int tid){
	
	t_tabla_tripulante* unaTabla;
	
	int mismo_id(t_tabla_tripulante* unaTable)
	{
		return ((unaTable->idTripulante) == tid );
	}
	
	pthread_mutex_lock(&mutexListaDeTripulantes);
	unaTabla = list_find(tablaDeSegmentosDeTripulantes, (void*) mismo_id);
	pthread_mutex_unlock(&mutexListaDeTripulantes);
	
	return unaTabla;
}

t_tcb* buscarTripulante(int idTripu){
	
	t_segmento* unSegmento;
	t_tabla_tripulante* unaTabla;
	t_tcb* unaTcb;
	
	unaTabla = buscarTablaTripulante(idTripu);
	
	unSegmento = unaTabla->seg_tcb;
	
	//ahora tengo que copiar el segmento en un tcb y copiar la tcb
	
	unaTcb = copiarSegmentacion(unSegmento);
	
	//mostrarTcb(unaTcb);
//	free(unaTabla);
//	free(unSegmento);
	
	return unaTcb;	
}

t_segmento* buscarSegmentoTripulante(int idTripu){
	
	t_segmento* unSegmento;
	t_tabla_tripulante* unaTabla;
	
	int mismo_id(t_tabla_tripulante* unaTabla)
	{
		return (idTripu == unaTabla->idTripulante);
	}
	
	//pthread_mutex_lock(&mutexListaDeTripulantes);
	unaTabla = list_find(tablaDeSegmentosDeTripulantes, (void*) mismo_id);
	//pthread_mutex_unlock(&mutexListaDeTripulantes);
	
	unSegmento = unaTabla->seg_tcb;
	
	
	
	return unSegmento;	
}


char* buscarTarea(int idPatota, int desplazamiento, int* esUltima){ //EL DESPLAZAMIENTO ES LA PROX INSTRUCCION A EJECUTAR DEL TRIPULANTE
	
	char* tareas = buscarArrayTareas(idPatota);
	
    char* tarea = separarTareas(tareas, desplazamiento, esUltima); //SEPARA LA TAREA Y ME DICE SI ES LA ULTIMA 

	return tarea;
}

char* buscarArrayTareas(int idPatota){
    t_tabla_patota* unaTabla;
	t_segmento* unSegmento;
	
	unaTabla = buscarTablaPatota(idPatota);
	
	//BUSCO EL SEGMENTO DE LAS TAREAS
	unSegmento = unaTabla->tareas;
	
	char* tareas = copiarSegmentacion(unSegmento); //ME DEVUELVE LAS TAREAS QUE SE ENCUENTRAN EN ESE SEGMENTO
	
    return tareas;
}



int buscarDirPatota(int idPatota){
    int direccionPatota;
    t_tabla_patota* tablaPatota;
    t_segmento* segmentoPCB;

    tablaPatota = buscarTablaPatota(idPatota);
    segmentoPCB = tablaPatota->pcb;

    direccionPatota = segmentoPCB->base;




    return direccionPatota;
}

t_tabla_patota* buscarTablaPatota(int idPatotaBuscada){
    t_tabla_patota* unaTabla;
	
	int mismo_id(t_tabla_patota* tablaPatota)
	{
		return ((tablaPatota->idPatota) == idPatotaBuscada);
	}
	pthread_mutex_lock(&mutexListaDePatotas);
	unaTabla = list_find(tablaDeSegmentosDePatotas, (void*) mismo_id);  //BUSCA EN LA LISTA DE SEGMENTOS DE PATOTAS, LA QUE TENGA EL MISMO ID
	pthread_mutex_unlock(&mutexListaDePatotas);
	return unaTabla;
}

t_segmento* buscarSegmentoSegunTamanio(int size){ 

    t_segmento* segmento;
    t_list* todosLosSegLibres;

    todosLosSegLibres =  buscarSegmentosDisponibles(); //PONE TODOS LOS SEGMENTOS VACIOS EN UNA LISTA 

    t_list* segmentosCandidatos;
    segmentosCandidatos = puedenGuardar( todosLosSegLibres , size); //ME DEVUELVE LOS SEGMENTOS QUE EL TIENEN ESPACIO NECESARIO PARA GUARDAR
    //log_info(logger,"Hay %d segmentos candidatos", list_size(segmentosCandidatos));
    if(list_is_empty(segmentosCandidatos)){
        log_info(logger,"No hay espacio suficiente para guardar, se debe compactar");
        compactacion();
        segmento = buscarSegmentoSegunTamanio(size);
    }else if(list_size(segmentosCandidatos)== 1){
        segmento = list_get(segmentosCandidatos, 0);
    }else{
        segmento = elegirSegCriterio(segmentosCandidatos, size); //SI EN LA LISTA HAY MAS DE UN SEGMENTO VA A ELEGIR EN QUE SEGMENTO LO VA A GUARDAR SEGUN EL CRITERIO
        

    }
    
    int mismoId(t_segmento* unSegmento)
    {
    	return (unSegmento->id == segmento->id);
    }
    
    list_remove_by_condition(todosLosSegLibres, (void*)mismoId); //Saco el segmento de la lista asi las puedo eliminar
    
	//list_destroy(todosLosSegLibres);
	//list_destroy(segmentosCandidatos);
	eliminarLista(todosLosSegLibres);
	list_destroy(segmentosCandidatos);
    
    return segmento;
}

t_list* puedenGuardar(t_list* segmentos, int size){
   
    t_list* aux;

    int puedoGuardarSeg(t_segmento* segmento){
        return(segmento->limite >= size);
    }
    aux= list_filter(segmentos, (void*)puedoGuardarSeg);

    return aux;
}

t_list* buscarSegmentosDisponibles(){
    
    t_list* segmentos = list_create();
    int base = 0;

    //YA SABEMOS QUE HAY LUGAR, SINO NO HUBIESE     
    
    while(base < (config_valores.tamanio_memoria)){
        t_segmento* unSegmento = buscarUnLugarLibre(&base);
        list_add(segmentos,unSegmento);
    }

    return segmentos;    
}

t_segmento* buscarUnLugarLibre(int* base){
    t_segmento* unSegmento = malloc(sizeof(t_segmento));
    int tamanio = 0;
    
    pthread_mutex_lock(&mutexBitMapSegment);
    if(bitarray_test_bit(bitMapSegment, *base) == 1){ //SI EL PRIMERO ES UN UNO, VA A CONTAR CUANDOS ESTAN OCUPADOS DESDE ESE Y CAMBIA LA BASE	
        int desplazamiento = contarEspaciosOcupadosDesde(bitMapSegment, *base); //CUENTA ESPACIOS OCUPADOS DESDE LA ABASE INDICADA
        *base += desplazamiento; //
    }
    pthread_mutex_unlock(&mutexBitMapSegment);
    // ACA YA LA BASE ESTA EN EL PRIMER 0 LIBRE
    tamanio = contarEspaciosLibresDesde(bitMapSegment, *base);  //TAMANIO ES EL TAMANIO DEL SEGMENTO CON 0 LIBRES //ACA MUERE
    
    unSegmento->id = generarId();
    unSegmento->base = *base;
    unSegmento->limite = tamanio;
    
    //MUEVO LA BASE PARA BUSCAR EL SIGUIENTE SEGMENTO DESPUES
    *base +=  tamanio;

    return unSegmento;
}

t_list* buscarSegmentosOcupados(){
    
    t_list* segmentos = list_create();
    
    agregarPatotasA(segmentos); //agrega todos los segmentos donde estan todas las patotas a la lista
    agregarTripusA(segmentos); // agrega todos los segmentos donde estan todos los tripus a la lista

    return segmentos; //UNIFICO LA LISTAS DE PATOTAS Y TRIPUS EN UNA SOLA 
}

void agregarPatotasA(t_list* segmentos){

	//pthread_mutex_lock(&mutexListaDeTripulantes);
    for(int i=0; i < list_size(tablaDeSegmentosDePatotas); i++){
        
        t_tabla_patota* tabla= list_get(tablaDeSegmentosDePatotas,i);
        list_add(segmentos, tabla->pcb);
        list_add(segmentos, tabla->tareas);
    }
   // pthread_mutex_unlock(&mutexListaDePatotas);
}

void agregarTripusA(t_list* segmentos){
    
	pthread_mutex_lock(&mutexListaDeTripulantes);
    for(int i =0; i < list_size(tablaDeSegmentosDeTripulantes); i++){
        
        t_tabla_tripulante* tabla = list_get(tablaDeSegmentosDeTripulantes, i);
        list_add(segmentos, tabla->seg_tcb); 
    }
    pthread_mutex_unlock(&mutexListaDeTripulantes);
}

//CRITERIOS DE ELECCION DE SEGMENTOS

t_segmento* elegirSegCriterio(t_list* segmentos, int size){

    t_segmento* segmento;
    
    log_info(logger,"Elijo segun %s", config_valores.criterio_seleccion);
    
    if(strcmp(config_valores.criterio_seleccion,"FF") == 0){//NO ENTRA LA CONCHA DE SU MADRE
        segmento = list_get(segmentos,0); //FIRST FIT DEVUELVE EL PRIMER SEGMENTO DONDE PUEDE GUARDARSE
    }else if(strcmp(config_valores.criterio_seleccion,"BF") == 0){
    	log_info(logger,"Entre por BF");
        segmento = segmentoBestFit(segmentos, size);
    }

    return segmento;
}

t_segmento* segmentoBestFit(t_list* segmentos, int size){

    t_segmento* segmento;

    int igualTamanio(t_segmento* segmento){
        return(segmento -> limite == size); //SEGMENTO QUE TENGA EL MISMO TAMANIO QUE LO QUE QUIERO GUARDAR
    }
    t_segmento* segmentoDeIgualTamanio = list_find(segmentos, (void*)igualTamanio); //ME DEVUELVE EL SEGMENTO DE MISMO TAMANIO

    if(segmentoDeIgualTamanio != NULL){
        segmento = segmentoDeIgualTamanio; 
    }else{      //SI NO ENCONTRE UN SEGMENTO DEL MISMO TAMANIO
        segmento = list_get_minimum(segmentos, (void*)segmentoMenorTamanio); //ME DEVUELVE EL SEGMENTO DE MENOR TAMANIO DONDE ENTRE LO QUE QUIERO GUARDAR
    }
    return segmento;
}

t_segmento* segmentoMenorTamanio(t_segmento* segmento, t_segmento* otro){ //TODO aca estaban sin asterisco

    if(segmento->limite < otro->limite){
        return segmento;
    }else
        return otro; 
}

//COMPACTACION

void compactacion(){
    //BUSCO SEGMENTOS OCUPADOS, CON EL BITMAP EN 1
    t_list* segmentosNoCompactados = buscarSegmentosOcupados();

    //SACO LO QUE TENGO GUARDADO EN ESOS SEGMENTOS,  Esta lista tiene PCB|TCB|Tareas|PCB|TCB|Tareas....
    t_list* cosasDeLosSegmentos = copiarContenidoSeg(segmentosNoCompactados);

    //LIMPIO EL BITMAP -> TODO EN 0
    liberarBitMap(bitMapSegment, 0, config_valores.tamanio_memoria);

    int cantidadS = list_size(segmentosNoCompactados);

    //NUEVA LISTA DE SEGMENTOS
    t_list* segmentosCompactados = list_create();

    //GUARDO LA LISTA COSASDELOSSEGMENTOS EN LA MEMORIA, EN ORDEN, TODOS PEGADOS
    for(int i =0 ; i<cantidadS ; i++){
        //AGARRO UNO
        t_segmento* miSegmento = list_get(segmentosNoCompactados, i);
        
        //LO GUARDO 
        t_segmento* nuevoSegmento = guardarElemento(list_get(cosasDeLosSegmentos,i), miSegmento->limite);
        
        //LO AGREGO A LA NUEVA LISTA
        list_add(segmentosCompactados, nuevoSegmento);
    }

    //ACTUALIZO LOS SEGMENTOS VIEJOS NO COMPACTADOS
    actualizarCompactacion(segmentosNoCompactados, segmentosCompactados);

    //LIBERO LISTAS
    eliminarLista(cosasDeLosSegmentos);   
    list_destroy(segmentosCompactados);
    list_destroy(segmentosNoCompactados);
    
}

t_list* copiarContenidoSeg(t_list* segmentosNoCompactados){
    t_list* segmentos = list_map(segmentosNoCompactados, (void*)copiarSegmentacion);
    
    return segmentos;
}

void actualizarCompactacion(t_list* segmentosNoCompactados,t_list*  segmentosCompactados){
    for(int i = 0; i<list_size(segmentosNoCompactados);i++){
        actualizarCadaSegmento(list_get(segmentosNoCompactados, i), list_get(segmentosCompactados,i));
    }
}

void actualizarCadaSegmento(t_segmento* segmentoViejo, t_segmento* segmentoNuevo){
	
	pthread_mutex_lock(&mutexMemoria);		
	log_info(logger, "El segmento %d ahora tiene base %p",segmentoViejo->id, memoriaPrincipal + segmentoNuevo->base);
	pthread_mutex_unlock(&mutexMemoria);
	
	actualizarListaPatotas(segmentoViejo, segmentoNuevo); //si esta lo cambia, sino no hace nada
	actualizarListaTripulantes(segmentoViejo, segmentoNuevo); //si esta lo cambia, sino no hace nada
	
	
}


void actualizarListaPatotas(t_segmento* segmentoViejo, t_segmento* segmentoNuevo){
	
	//primero busco en la listaDePatotas al segmento viejo
	
	int tieneSegmento(t_tabla_patota* tabla)
	{
		return (tabla->pcb->id == segmentoViejo->id) || (tabla->tareas->id == segmentoViejo->id);
	}
	
	//pthread_mutex_lock(&mutexListaDePatotas);
	if(list_find(tablaDeSegmentosDePatotas, (void*) tieneSegmento) != NULL)
	{
		t_tabla_patota* unaTabla = list_find(tablaDeSegmentosDePatotas, (void*) tieneSegmento);
		if(unaTabla->pcb->id == segmentoViejo->id)
		{
			actualizarBase(unaTabla->pcb, segmentoNuevo);
		}
		else if(unaTabla->tareas->id == segmentoViejo->id)
		{
			actualizarBase(unaTabla->tareas, segmentoNuevo);
		}
		
		free(segmentoNuevo);
	} 
	//pthread_mutex_unlock(&mutexListaDePatotas);
		
	
}


void actualizarListaTripulantes(t_segmento* segmentoViejo, t_segmento* segmentoNuevo){
	
	//primero busco en la listaDePatotas al segmento viejo
		
		int tieneSegmento(t_tabla_tripulante* tabla)
		{
			return (tabla->seg_tcb->id == segmentoViejo->id);
		}
		//pthread_mutex_lock(&mutexListaDeTripulantes);
		if(list_find(tablaDeSegmentosDeTripulantes, (void*) tieneSegmento) != NULL)
		{					
			t_tabla_tripulante* unaTabla = list_find(tablaDeSegmentosDeTripulantes, (void*) tieneSegmento);
			
			if(unaTabla->seg_tcb->id == segmentoViejo->id)//aca rompe tengo q ir a hacer el tp de eco
			{
				actualizarBase(unaTabla->seg_tcb, segmentoNuevo);
			}
		free(segmentoNuevo);
		}
		//pthread_mutex_unlock(&mutexListaDeTripulantes);
		
	
	
}



void actualizarBase(t_segmento* segmento, t_segmento* otroSeg){

    segmento->base= otroSeg->base;
}

void s_actualizarTripulante(t_tcb* unaTcb){
    t_tabla_tripulante* unaTabla;
    t_segmento* unSegmento;

    unaTabla = buscarTablaTripulante(unaTcb->idTripulante);
    unSegmento = unaTabla->seg_tcb;

    guardarEnMemoria(unaTcb, unSegmento,21); //TODO ANTES EN VEZ DE UNSEGMENTO DECIA SEG_TCB
}

//DUMP

void dumpSegmentacion(){
    FILE * file;

	char* nombreDeArchivo = dameNombre();
	
	log_info(logger,"Creo un archivo con el nombre %s\n", nombreDeArchivo);
	
	char* time = temporal_get_string_time("%d/%m/%y %H:%M:%S");
	
	
	file = fopen(nombreDeArchivo, "w");
	
	fprintf(file, "\n\n");
	fprintf(file, "- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - \n");
	fprintf(file, "Dump: %s\n\n",time);
	
	mostrarProcesosSegmentacion(file);
	
	fprintf(file, "- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - \n");
	
	//Proceso: 1	Segmento: 1	Inicio: 0x0000	Tam: 20b
	
	fclose(file);
	
	free(nombreDeArchivo);
	free(time);
}

void mostrarProcesosSegmentacion(FILE* file){
	
	void pasarFile(t_tabla_patota* tabla)
	{
		mostrarProceso(tabla, file);
	}
	
	//pthread_mutex_lock(&mutexListaDePatotas);
	if(list_is_empty(tablaDeSegmentosDePatotas)){
		fprintf(file, "LA MEMORIA ESTA VACIA\n");
		log_info(logger, "No hay ningun proceso mi rey, comprate una vida pa");
	}
	else
	{
		list_iterate(tablaDeSegmentosDePatotas, (void*)pasarFile);		
	}
	//pthread_mutex_unlock(&mutexListaDePatotas);
}

void mostrarProceso(t_tabla_patota* unaTabla, FILE* file){
	
	//muestro en este orden: pcb, tareas, tripulantes
	
	fprintf(file,"Segmentos de la patota %d\n\n", unaTabla->idPatota);
	
	mostrarSegmentoDump(unaTabla->idPatota, unaTabla->tareas, file);
	mostrarSegmentoDump(unaTabla->idPatota, unaTabla->pcb, file);
	
	fprintf(file,"\n");
	//Busco los tripulantes de la patota
	
	fprintf(file, "Tripulantes de la patota %d:\n\n",unaTabla->idPatota);
	
	mostrarSegmentosTripulantes(unaTabla->idPatota, file);
	
	fprintf(file,"\n\n");
}

void mostrarSegmentoDump(int pid, t_segmento* unSegmento, FILE* file){
	
	//Proceso: 1	Segmento: 1	Inicio: 0x0000	Tam: 20b
	pthread_mutex_lock(&mutexMemoria);	
	fprintf(file, "Proceso: %d    Segmento: %d        Inicio: %p    Tam: %db \n", pid, unSegmento->id, (memoriaPrincipal + unSegmento->base), unSegmento->limite);
	pthread_mutex_unlock(&mutexMemoria);	
}

void mostrarSegmentosTripulantes(int pid, FILE* file){
	
	int estaEnPatota(t_tabla_tripulante* unaTabla)
	{
		t_tcb* unaTcb = buscarTripulante(unaTabla->idTripulante);
		int idPatota = unaTcb->puntero_pcb;
		free(unaTcb);
		return idPatota == pid;		
	}
	
	void mostrarSegmentoTripu(t_tabla_tripulante* tabla){
		
		mostrarSegmentoDump(pid,tabla->seg_tcb,file);
		
	}
	
	//pthread_mutex_lock(&mutexListaDeTripulantes);
	t_list* listaFiltrada = list_filter(tablaDeSegmentosDeTripulantes,(void*)estaEnPatota);
	
	list_iterate(listaFiltrada, (void*) mostrarSegmentoTripu);
	
	//eliminarLista(listaFiltrada);
	list_destroy(listaFiltrada);
	//pthread_mutex_unlock(&mutexListaDeTripulantes);
}

void liberarTabla(t_tabla_tripulante* unaTabla){
	
	free(unaTabla->seg_tcb);
	free(unaTabla);	
}

void mostrarTripulante(int id){
	
	t_tcb* unaTcb = buscarTripulante(id);
	
	mostrarTcb(unaTcb);
	
	free(unaTcb);
	
	
}

void liberarMemoriaSegmentacion(){
	
	bitarray_destroy(bitMapSegment);
	free(datos);
	list_destroy_and_destroy_elements(tablaDeSegmentosDePatotas, (void*)eliminarTablaPatota);
	list_destroy_and_destroy_elements(tablaDeSegmentosDeTripulantes, (void*)eliminarTablaTripulante);


	
}

void eliminarTablaPatota(t_tabla_patota* unaTabla){
	
	free(unaTabla->pcb);
	free(unaTabla->tareas);
	free(unaTabla); 
	
	
}

void eliminarTablaTripulante(t_tabla_tripulante* unaTabla){
	
	free(unaTabla->seg_tcb);
	free(unaTabla);
	
}

void sumarTripu(int idPato){
	
	t_tabla_patota* unaTabla = buscarTablaPatota(idPato);
	
	pthread_mutex_lock(&mutexVivos);
	unaTabla->tripusVivos ++;
	pthread_mutex_unlock(&mutexVivos);
	
}

void restarTripu(int idPato){
	
	t_tabla_patota* unaTabla = buscarTablaPatota(idPato);
	
	unaTabla->tripusVivos --;
	
	if(unaTabla->tripusVivos == 0){//Si me quede sin tripulantes la borro a la basura relampagos y centellas
		
		//Saco la pcb
		liberarBitMap(bitMapSegment, unaTabla->pcb->base, unaTabla->pcb->limite);
		
		//Saco las tareas
		liberarBitMap(bitMapSegment, unaTabla->tareas->base, unaTabla->tareas->limite );
		
		//Elimino la tabla
		
		int mismo_id(t_tabla_patota* unaTabla)
		{
			return (unaTabla->idPatota == idPato); 
		}
		
		list_remove_and_destroy_by_condition(tablaDeSegmentosDePatotas, (void*)mismo_id, (void*)eliminarTablaPatota);
		
		
		log_info(logger, "Se elimino la patota %d",idPato);
		
	}
	
	
	
}




