#include "sabotajes.h"

void solucionarSabotajes() {
    // revisar que esta roto
	int fd = open(superBloque_file_path, O_CREAT | O_RDWR, 0664);
    
    void* superBloque = mmap(NULL, 8, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    uint32_t tamanioBloque;
    memcpy(&tamanioBloque, superBloque, sizeof(int));
	
    uint32_t cantidadDeBloquesQueDeberiaHaber;
    memcpy(&cantidadDeBloquesQueDeberiaHaber, sizeof(int)+superBloque, sizeof(int));

    int tamanioBlocks = contarBlock();

    //t_config* superbloque_file = config_create(metadata_file_path_superbloque); NO LO BORRO PQ ME QUEDE ¿?
    // PERO SI ES LO QUE CREO LA RESPUESTA ES QUE EL SUPERBLOQUE NO SE PUEDE ABRIR COMO UNA CONFIG

    int cantidadRealDeBloques = tamanioBlocks / tamanioBloque;

    if(cantidadRealDeBloques != cantidadDeBloquesQueDeberiaHaber) {
        cantidadDeBloquesQueDeberiaHaber = (uint32_t) cantidadRealDeBloques;
        void* uints= malloc(4);
        uints= realloc(uints, sizeof(int));
        memcpy(uints, &cantidadDeBloquesQueDeberiaHaber, sizeof(int));
        memcpy(superBloque+sizeof(int), uints, sizeof(int));
        msync(superBloque, sizeof(int)*2, MS_SYNC);
        close(fd);
        free(uints);
        // actualizar el valor en la metadata,  creo que se hace con set                              AYUDA ACA FRAN!

        return;
    }
    
    t_config* oxigeno_file = config_create(metadata_file_path_oxigeno);

        int cantidadDeOEscritas =  config_get_int_value(oxigeno_file, "SIZE");
        int cantBloquesOOO = config_get_int_value(oxigeno_file, "BLOCK_COUNT");
        char** bloquesUsadosOOO = config_get_array_value(oxigeno_file, "BLOCKS");

    // evaluar que los bloques en mi lista sean validos, ninguno negativo ni mayor al limite
    // arreglar los que esten mal y volver a escribir todas los caracteres
    int habiaAlgunBloqueConUnNumeroInvalido = 0;
    int bloquesLen=-1;
    while (bloquesUsadosOOO[++bloquesLen] != NULL){}
    
    //bloquesLen++;

    if(bloquesLen == cantBloquesOOO) {
        for (int i=0; i < cantBloquesOOO; i++) {
            int bloqueAAnalizarOOO = atoi(bloquesUsadosOOO[i]);
            if(bloqueAAnalizarOOO < 0 || bloqueAAnalizarOOO > cantidadRealDeBloques) {
                bloquesUsadosOOO[i] = string_itoa(buscarPrimerBloqueLibre());
                habiaAlgunBloqueConUnNumeroInvalido = 1;
            }
        }
    }
    // arreglar los que esten mal, si vuelvo a escribir algun bloque,
    // los caracteres que no haya escrito recien de este bloque los completo con basura

    if (habiaAlgunBloqueConUnNumeroInvalido) {
    	int cantidadEscrita=0;
    	int i=0;
    	while(cantidadEscrita<cantidadDeOEscritas){

    	int bloqueAUsar = atoi(bloquesUsadosOOO[i]);
    	int escritasEnBloque=0;
    	while(escritasEnBloque<tamanioBloque && cantidadEscrita<cantidadDeOEscritas){

            pthread_mutex_lock(&sincro_block);
            memcpy(copiaBlock + bloqueAUsar * tamanioBloque + (cantidadEscrita % tamanioBloque), "O", sizeof(char));
            pthread_mutex_unlock(&sincro_block); //VER EN LOS OTROS DE ESCRIBIR LA OTRA LETRA
            cantidadEscrita++;
            escritasEnBloque++;
    	}
    	i++;
    	}
    	char* actualizarBloques=string_new();
    	string_append(&actualizarBloques,"[");
    	for(int i=0; i<cantBloquesOOO; i++) {
    	    if(i==0)
    	    string_append(&actualizarBloques,bloquesUsadosOOO[i]);
    	    else{
    	     string_append(&actualizarBloques,",");
    	     string_append(&actualizarBloques,bloquesUsadosOOO[i]);
    	   }

    	}
    	string_append(&actualizarBloques,"]");
        config_set_value(oxigeno_file,"BLOCKS",actualizarBloques);
        config_save(oxigeno_file);
        config_destroy(oxigeno_file);
        free(bloquesUsadosOOO);

    	return;
    }

    if(bloquesLen != cantBloquesOOO) {
        cantBloquesOOO = bloquesLen;
        char* actualizarCantOOO = string_itoa(cantBloquesOOO);
	    config_set_value(oxigeno_file,"BLOCK_COUNT",actualizarCantOOO);
        config_save(oxigeno_file);
        config_destroy(oxigeno_file);
        free(actualizarCantOOO);
        free(bloquesUsadosOOO);
        return;
    }
    if(cantBloquesOOO>1){
        // revisar que estan en orden
            int bloqueNoCompleto = -1;
            for (int i=0; i< cantBloquesOOO; i++) {
                int primerEspacio = primerEspacioLibre(atoi(bloquesUsadosOOO[i]), 'O');
                if(primerEspacio != -1) {
                    bloqueNoCompleto = i;
                }
            }
            // si bloqueNoCompleto es =! -1
                // filtrar la lista para que no este incluido el bloqueNoCompleto
                // crear lista nueva sin ese numero
                // agregar al final bloqueNoCompleto
            if(bloqueNoCompleto != -1  && bloqueNoCompleto != (cantBloquesOOO-1)) {
                char* aux= bloquesUsadosOOO[cantBloquesOOO-1];
                bloquesUsadosOOO[cantBloquesOOO-1]= bloquesUsadosOOO[bloqueNoCompleto];
                bloquesUsadosOOO[bloqueNoCompleto] = aux;
                char* bloquesNuevos= string_new();
                string_append(&bloquesNuevos,"[");
                for(int i=0; i<cantBloquesOOO; i++) {
                	if(i==0)
                	string_append(&bloquesNuevos,bloquesUsadosOOO[i]);
                	else{
                		string_append(&bloquesNuevos,",");
                		string_append(&bloquesNuevos,bloquesUsadosOOO[i]);
                	}

                }
                string_append(&bloquesNuevos,"]");
                config_set_value(oxigeno_file,"BLOCKS",bloquesNuevos);
                config_save(oxigeno_file);
                config_destroy(oxigeno_file);
                free(bloquesNuevos);
                free(bloquesUsadosOOO);
                return;
            }
            
            
            
            
        // revisar size
        int cantidadDeBloquesLlenosOOO = (cantBloquesOOO - 1) * tamanioBloque;
        int cantidadDeOEnELUltimoBloque;
        int ultimoBloqueOOO = atoi(bloquesUsadosOOO[cantBloquesOOO-1]);

        if(primerEspacioLibre(ultimoBloqueOOO, 'O') == -1)
        {
            cantidadDeOEnELUltimoBloque = tamanioBlocks;
        } 
        else
        {
            cantidadDeOEnELUltimoBloque = primerEspacioLibre(ultimoBloqueOOO, 'O');
        }

        int tamanioPostaMedidoOOO = cantidadDeBloquesLlenosOOO + cantidadDeOEnELUltimoBloque;

        if(tamanioPostaMedidoOOO != cantidadDeOEscritas) {
            char * cantidadDeOEscritasActualiz = string_itoa(tamanioPostaMedidoOOO);
            config_set_value(oxigeno_file,"SIZE",cantidadDeOEscritasActualiz);
            config_save(oxigeno_file);
                    config_destroy(oxigeno_file);
                    free(bloquesUsadosOOO);
            return;
        }
    } else if(cantBloquesOOO==1) {
		int cantidadDeOEnELUltimoBloqueB;
		int ultimoBloqueBBB = atoi(bloquesUsadosOOO[cantBloquesOOO-1]);
        if(primerEspacioLibre(ultimoBloqueBBB, 'O') == -1) {
        	cantidadDeOEnELUltimoBloqueB = tamanioBlocks;
        } else {
        	cantidadDeOEnELUltimoBloqueB = primerEspacioLibre(ultimoBloqueBBB, 'O') ;
        }
        int tamanioPostaMedidoBBB =  cantidadDeOEnELUltimoBloqueB;
        if(tamanioPostaMedidoBBB != cantidadDeOEscritas) {
            char * cantidadDeBEscritasActualiz = string_itoa(tamanioPostaMedidoBBB);
            config_set_value(oxigeno_file,"SIZE",cantidadDeBEscritasActualiz);
            config_save(oxigeno_file);
                    config_destroy(oxigeno_file);
                    free(bloquesUsadosOOO);
            return;
	}
}
    else{
            	config_set_value(oxigeno_file,"SIZE","0");
            	config_save(oxigeno_file);
            }
    free(bloquesUsadosOOO);
    config_destroy(oxigeno_file);
    // HACER LO MISMO PARA BASURA.

    t_config* basura_file = config_create(metadata_file_path_basura);

            int cantidadDeBEscritas =  config_get_int_value(basura_file, "SIZE");
            int cantBloquesBBB = config_get_int_value(basura_file, "BLOCK_COUNT");
            char** bloquesUsadosBBB = config_get_array_value(basura_file, "BLOCKS");

        // evaluar que los bloques en mi lista sean validos, ninguno negativo ni mayor al limite
        // arreglar los que esten mal y volver a escribir todas los caracteres
        habiaAlgunBloqueConUnNumeroInvalido = 0;
        bloquesLen=-1;
        while (bloquesUsadosBBB[++bloquesLen] != NULL){}

        if(bloquesLen == cantBloquesBBB) {
            for (int i=0; i < cantBloquesBBB; i++) {
                int bloqueAAnalizarBBB = atoi(bloquesUsadosBBB[i]);
                if(bloqueAAnalizarBBB < 0 || bloqueAAnalizarBBB > cantidadRealDeBloques) {
                	bloquesUsadosBBB[i] = string_itoa(buscarPrimerBloqueLibre());
                    habiaAlgunBloqueConUnNumeroInvalido = 1;
                }
            }
        }
        // arreglar los que esten mal, si vuelvo a escribir algun bloque,
        // los caracteres que no haya escrito recien de este bloque los completo con basura

        if (habiaAlgunBloqueConUnNumeroInvalido) {
        	int cantidadEscrita=0;
        	int i=0;
        	while(cantidadEscrita<cantidadDeBEscritas){

        	int bloqueAUsar = atoi(bloquesUsadosBBB[i]);
        	int escritasEnBloque=0;
        	while(escritasEnBloque<tamanioBloque && cantidadEscrita<cantidadDeBEscritas){

                pthread_mutex_lock(&sincro_block);
                memcpy(copiaBlock + bloqueAUsar * tamanioBloque + (cantidadEscrita % tamanioBloque), "B", sizeof(char));
                pthread_mutex_unlock(&sincro_block);
                cantidadEscrita++;
                escritasEnBloque++;
        	}
        	i++;
        	}
        	char* actualizarBloques=string_new();
        	string_append(&actualizarBloques,"[");
        	for(int i=0; i<cantBloquesBBB; i++) {
        	    if(i==0)
        	    string_append(&actualizarBloques,bloquesUsadosBBB[i]);
        	    else{
        	     string_append(&actualizarBloques,",");
        	     string_append(&actualizarBloques,bloquesUsadosBBB[i]);
        	   }

        	}
        	string_append(&actualizarBloques,"]");
            config_set_value(basura_file,"BLOCKS",actualizarBloques);
            config_save(basura_file);
            config_destroy(basura_file);
            free(bloquesUsadosBBB);

        	return;
        }

        if(bloquesLen != cantBloquesBBB) {
        	cantBloquesBBB = bloquesLen;
            char* actualizarCantBBB = string_itoa(cantBloquesBBB);
    	    config_set_value(oxigeno_file,"BLOCK_COUNT",actualizarCantBBB);
            config_save(basura_file);
            config_destroy(basura_file);
            free(actualizarCantBBB);
            free(bloquesUsadosBBB);
            return;
        }

        if(cantBloquesBBB>1){
        // revisar que estan en orden
            int bloqueNoCompleto = -1;
            for (int i=0; i< cantBloquesBBB; i++) {
                int primerEspacio = primerEspacioLibre(atoi(bloquesUsadosBBB[i]), 'B');
                if(primerEspacio != -1) {
                    bloqueNoCompleto = i;
                }
            }
            // si bloqueNoCompleto es =! -1
                // filtrar la lista para que no este incluido el bloqueNoCompleto
                // crear lista nueva sin ese numero
                // agregar al final bloqueNoCompleto
            if(bloqueNoCompleto != -1  && bloqueNoCompleto != (cantBloquesBBB-1)) {
                char* aux= bloquesUsadosBBB[cantBloquesBBB-1];
                bloquesUsadosBBB[cantBloquesBBB-1]= bloquesUsadosBBB[bloqueNoCompleto];
                bloquesUsadosBBB[bloqueNoCompleto] = aux;
                char* bloquesNuevos= string_new();
                string_append(&bloquesNuevos,"[");
                for(int i=0; i<cantBloquesBBB; i++) {
                	if(i==0)
                	string_append(&bloquesNuevos,bloquesUsadosBBB[i]);
                	else{
                		string_append(&bloquesNuevos,",");
                		string_append(&bloquesNuevos,bloquesUsadosBBB[i]);
                	}

                }
                string_append(&bloquesNuevos,"]");
                config_set_value(basura_file,"BLOCKS",bloquesNuevos);
                config_save(basura_file);
                config_destroy(basura_file);
                free(bloquesNuevos);
                free(bloquesUsadosBBB);
                return;
            }
        int cantidadDeBloquesLlenosBBB = (cantBloquesBBB - 1) * tamanioBloque;
        int cantidadDeOEnELUltimoBloqueB;
        int ultimoBloqueBBB = atoi(bloquesUsadosBBB[cantBloquesBBB-1]);

        if(primerEspacioLibre(ultimoBloqueBBB, 'B') == -1) {
        	cantidadDeOEnELUltimoBloqueB = tamanioBlocks;
        } else {
        	cantidadDeOEnELUltimoBloqueB = primerEspacioLibre(ultimoBloqueBBB, 'B') ;
        }

        int tamanioPostaMedidoBBB = cantidadDeBloquesLlenosBBB + cantidadDeOEnELUltimoBloqueB;

        if(tamanioPostaMedidoBBB != cantidadDeBEscritas) {
            char * cantidadDeBEscritasActualiz = string_itoa(tamanioPostaMedidoBBB);
            config_set_value(basura_file,"SIZE",cantidadDeBEscritasActualiz);
            config_save(basura_file);
                    config_destroy(basura_file);
                    free(bloquesUsadosBBB);
            return;
        }
        }else if(cantBloquesBBB==1){
        		int cantidadDeOEnELUltimoBloqueB;
        		int ultimoBloqueBBB = atoi(bloquesUsadosBBB[cantBloquesBBB-1]);
                if(primerEspacioLibre(ultimoBloqueBBB, 'B') == -1) {
                	cantidadDeOEnELUltimoBloqueB = tamanioBlocks;
                } else {
                	cantidadDeOEnELUltimoBloqueB = primerEspacioLibre(ultimoBloqueBBB, 'B') ;
                }
                int tamanioPostaMedidoBBB =  cantidadDeOEnELUltimoBloqueB;
                if(tamanioPostaMedidoBBB != cantidadDeBEscritas) {
                    char * cantidadDeBEscritasActualiz = string_itoa(tamanioPostaMedidoBBB);
                    config_set_value(basura_file,"SIZE",cantidadDeBEscritasActualiz);
                    config_save(basura_file);
                            config_destroy(basura_file);
                            free(bloquesUsadosBBB);
                    return;
        	}
        }else{
        	config_set_value(basura_file,"SIZE","0");
        	config_save(basura_file);
        }
        free(bloquesUsadosBBB);
        config_destroy(basura_file);

        // HACER LO MISMO PARA COMIDA

        t_config* comida_file = config_create(metadata_file_path_comida);

                int cantidadDeCEscritas =  config_get_int_value(comida_file, "SIZE");
                int cantBloquesCCC = config_get_int_value(comida_file, "BLOCK_COUNT");
                char** bloquesUsadosCCC = config_get_array_value(comida_file, "BLOCKS");

            // evaluar que los bloques en mi lista sean validos, ninguno negativo ni mayor al limite
            // arreglar los que esten mal y volver a escribir todas los caracteres
            habiaAlgunBloqueConUnNumeroInvalido = 0;
            bloquesLen=-1;
            while (bloquesUsadosCCC[++bloquesLen] != NULL){}

            if(bloquesLen == cantBloquesCCC) {
                for (int i=0; i < cantBloquesCCC; i++) {
                    int bloqueAAnalizarCCC = atoi(bloquesUsadosCCC[i]);
                    if(bloqueAAnalizarCCC < 0 || bloqueAAnalizarCCC > cantidadRealDeBloques) {
                    	bloquesUsadosCCC[i] = string_itoa(buscarPrimerBloqueLibre());
                        habiaAlgunBloqueConUnNumeroInvalido = 1;
                    }
                }
            }
            // arreglar los que esten mal, si vuelvo a escribir algun bloque,
            // los caracteres que no haya escrito recien de este bloque los completo con basura

            if (habiaAlgunBloqueConUnNumeroInvalido) {
            	int cantidadEscrita=0;
            	int i=0;
            	while(cantidadEscrita<cantidadDeCEscritas){

            	int bloqueAUsar = atoi(bloquesUsadosCCC[i]);
            	int escritasEnBloque=0;
            	while(escritasEnBloque<tamanioBloque && cantidadEscrita<cantidadDeCEscritas){

                    pthread_mutex_lock(&sincro_block);
                    memcpy(copiaBlock + bloqueAUsar * tamanioBloque + (cantidadEscrita % tamanioBloque), "C", sizeof(char));
                    pthread_mutex_unlock(&sincro_block);
                    cantidadEscrita++;
                    escritasEnBloque++;
            	}
            	i++;
            	}
            	char* actualizarBloques=string_new();
            	string_append(&actualizarBloques,"[");
            	for(int i=0; i<cantBloquesCCC; i++) {
            	    if(i==0)
            	    string_append(&actualizarBloques,bloquesUsadosCCC[i]);
            	    else{
            	     string_append(&actualizarBloques,",");
            	     string_append(&actualizarBloques,bloquesUsadosCCC[i]);
            	   }

            	}
            	string_append(&actualizarBloques,"]");
                config_set_value(comida_file,"BLOCKS",actualizarBloques);
                config_save(comida_file);
                config_destroy(comida_file);
                free(bloquesUsadosCCC);

            	return;
            }

            if(bloquesLen != cantBloquesCCC) {
            	cantBloquesCCC = bloquesLen;
                char* actualizarCantCCC = string_itoa(cantBloquesCCC);
        	    config_set_value(comida_file,"BLOCK_COUNT",actualizarCantCCC);
                config_save(comida_file);
                config_destroy(comida_file);
                free(bloquesUsadosCCC);
                return;
            }
            if(cantBloquesCCC>1){
                // revisar que estan en orden
            int bloqueNoCompleto = -1;
            for (int i=0; i< cantBloquesCCC; i++) {
                int primerEspacio = primerEspacioLibre(atoi(bloquesUsadosCCC[i]), 'C');
                if(primerEspacio != -1) {
                    bloqueNoCompleto = i;
                }
            }
            // si bloqueNoCompleto es =! -1
                // filtrar la lista para que no este incluido el bloqueNoCompleto
                // crear lista nueva sin ese numero
                // agregar al final bloqueNoCompleto
            if(bloqueNoCompleto != -1  && bloqueNoCompleto != (cantBloquesCCC-1)) {
                char* aux= bloquesUsadosCCC[cantBloquesCCC-1];
                bloquesUsadosCCC[cantBloquesCCC-1]= bloquesUsadosCCC[bloqueNoCompleto];
                bloquesUsadosCCC[bloqueNoCompleto] = aux;
                char* bloquesNuevos= string_new();
                string_append(&bloquesNuevos,"[");
                for(int i=0; i<cantBloquesCCC; i++) {
                	if(i==0)
                	string_append(&bloquesNuevos,bloquesUsadosCCC[i]);
                	else{
                		string_append(&bloquesNuevos,",");
                		string_append(&bloquesNuevos,bloquesUsadosCCC[i]);
                	}

                }
                string_append(&bloquesNuevos,"]");
                config_set_value(comida_file,"BLOCKS",bloquesNuevos);
                config_save(comida_file);
                config_destroy(comida_file);
                free(bloquesNuevos);
                free(bloquesUsadosCCC);
                return;
            }
            int cantidadDeBloquesLlenosCCC = (cantBloquesCCC - 1) * tamanioBloque;
            int cantidadDeCEnELUltimoBloque;
            int ultimoBloqueCCC = atoi(bloquesUsadosCCC[cantBloquesCCC-1]);

            if(primerEspacioLibre(ultimoBloqueCCC, 'C') == -1) {
            	cantidadDeCEnELUltimoBloque = tamanioBlocks;
            } else {
            	cantidadDeCEnELUltimoBloque = primerEspacioLibre(ultimoBloqueCCC, 'C');
            }

            int tamanioPostaMedidoCCC = cantidadDeBloquesLlenosCCC + cantidadDeCEnELUltimoBloque;

            if(tamanioPostaMedidoCCC != cantidadDeCEscritas) {
                char * cantidadDeCEscritasActualiz = string_itoa(tamanioPostaMedidoCCC);
                config_set_value(comida_file,"SIZE",cantidadDeCEscritasActualiz);
                config_save(comida_file);
                        config_destroy(comida_file);
                free(bloquesUsadosCCC);
                return;
            }
            }else if(cantBloquesCCC==1){
        		int cantidadDeCEnELUltimoBloqueC;
        		int ultimoBloqueCCC = atoi(bloquesUsadosCCC[cantBloquesCCC-1]);
                if(primerEspacioLibre(ultimoBloqueCCC, 'C') == -1) {
                	cantidadDeCEnELUltimoBloqueC = tamanioBlocks;
                } else {
                	cantidadDeCEnELUltimoBloqueC = primerEspacioLibre(ultimoBloqueCCC, 'C') ;
                }
                int tamanioPostaMedidoCCC =  cantidadDeCEnELUltimoBloqueC;
                if(tamanioPostaMedidoCCC != cantidadDeCEscritas) {
                    char * cantidadDeCEscritasActualiz = string_itoa(tamanioPostaMedidoCCC);
                    config_set_value(comida_file,"SIZE",cantidadDeCEscritasActualiz);
                    config_save(comida_file);
                    config_destroy(comida_file);
                    free(bloquesUsadosCCC);
                    return;
        	}
        }    else{
        	config_set_value(comida_file,"SIZE","0");
        	config_save(comida_file);
        }
            free(bloquesUsadosCCC);
            config_destroy(comida_file);


    // SI LLEGUE HASTA ACA, PUEDO ASEGURAR QUE LO QUE ESTA MAL ES EL BITMAP

            t_config* comida_file1 = config_create(metadata_file_path_oxigeno);
            int cantBloquesC = config_get_int_value(comida_file1, "BLOCK_COUNT");
            char** bloquesUsadosCCC1 = config_get_array_value(comida_file1, "BLOCKS");

            t_config* basura_file1 = config_create(metadata_file_path_basura);
            int cantBloquesB = config_get_int_value(basura_file1, "BLOCK_COUNT");
            char** bloquesUsadosBBB1 = config_get_array_value(basura_file1, "BLOCKS");

            t_config* oxigeno_file1 = config_create(metadata_file_path_comida);
            int cantBloquesO = config_get_int_value(oxigeno_file1, "BLOCK_COUNT");
            char** bloquesUsadosOOO1 = config_get_array_value(oxigeno_file1, "BLOCKS");

            int cantidadDeBitacorasAabrir=0;
            char* id= string_itoa(cantidadDeBitacorasAabrir+1);

            int validar=1;
            while(validar){
            	free(id); //TODO VER SI FUNCA
            	id= string_itoa(cantidadDeBitacorasAabrir+1);
            	char* bitacora_tripu = string_new();
            	 string_append(&bitacora_tripu, bitacora_path);
            	string_append(&bitacora_tripu, "Tripulante");
            	 string_append(&bitacora_tripu, id);
            	string_append(&bitacora_tripu, ".ims");
            	if(existe_archivo(bitacora_tripu)){
            		cantidadDeBitacorasAabrir++;
            	}else
            		validar=0;

            	free(bitacora_tripu);
            }

            reiniciarBitmapEnCero(cantidadRealDeBloques);

            //ASIGNO LOS BITS DEL OXIGENO
            for(int i=0; i<cantBloquesO;i++){
            	bitarray_set_bit(bitmap,atoi(bloquesUsadosOOO1[i]));
            }
            //ASIGNO LOS BITS DE LA BASURA
             for(int i=0; i<cantBloquesB;i++){
                bitarray_set_bit(bitmap,atoi(bloquesUsadosBBB1[i]));
             }
             //ASIGNO LOS BITS DE LA COMIDA
              for(int i=0; i<cantBloquesC;i++){
                 bitarray_set_bit(bitmap,atoi(bloquesUsadosCCC1[i]));
              }

              msync(bitmap->bitarray, tamanoBitmap, MS_SYNC);
              //ASIGNO LOS BITS DE CADA PUTA BITACORA
              abrirBitacoras(cantidadDeBitacorasAabrir);
              msync(bitmap->bitarray, tamanoBitmap, MS_SYNC);

              config_destroy(comida_file1);
              config_destroy(basura_file1);
              config_destroy(oxigeno_file1);
              free(bloquesUsadosCCC1);
              free(bloquesUsadosBBB1);
              free(bloquesUsadosOOO1);
              free(id);

    /*int* nuevoBitmap [cantidadRealDeBloques];

    // REVISAR QUE BLOQUES ESTAN USANDO LAS BITACORAS


    for (int i; i <= cantBloquesOOO; i++) {
            int bloqueAActualizarEnBitmap = atoi(bloquesUsadosOOO[i]);
            nuevoBitmap[bloqueAActualizarEnBitmap] = 1;
    }

    for (int i; i <= cantBloquesBBB; i++) {
            int bloqueAActualizarEnBitmap = atoi(bloquesUsadosBBB[i]);
            nuevoBitmap[bloqueAActualizarEnBitmap] = 1;
    }

    for (int i; i <= cantBloquesCCC; i++) {
            int bloqueAActualizarEnBitmap = atoi(bloquesUsadosCCC[i]);
            nuevoBitmap[bloqueAActualizarEnBitmap] = 1;
    }*/


   return;
}

void reiniciarBitmapEnCero(int cantidadRealDeBloques){
	for(int i=0;i<cantidadRealDeBloques;i++){
		bitarray_clean_bit(bitmap,i);
	}
	msync(bitmap->bitarray, tamanoBitmap, MS_SYNC);
	return;
}

void abrirBitacoras(int cantidad){
	int abiertas=0;
	while(abiertas<cantidad){
		int bloquesLen = -1;
        char* id= string_itoa(abiertas+1);
		char* bitacora_tripu = string_new();
		  string_append(&bitacora_tripu, bitacora_path);
		  string_append(&bitacora_tripu, "Tripulante");
		  string_append(&bitacora_tripu, id);
		 string_append(&bitacora_tripu, ".ims");
		 free(id);
		 t_config* bitacora_config = config_create(bitacora_tripu);
		  char** bloquesUsados = config_get_array_value(bitacora_config, "BLOCKS");
		 while (bloquesUsados[++bloquesLen] != NULL)
			 //pasar los bits del numero de bloques a 1
			 for(int i=0;i<=bloquesLen;i++){
				 int bloque=atoi(bloquesUsados[i]);
				 bitarray_set_bit(bitmap,bloque);


					}

		 config_destroy(bitacora_config);
		abiertas++;
		free(bloquesUsados);
		free(bitacora_tripu);
	}

	msync(bitmap->bitarray, tamanoBitmap, MS_SYNC);

}

int contarBlock(){
	/*int tamanioBlocks=0;
	while(copiaBlock+ tamanioBlocks != NULL)  QUE SE YO, NO ANDAB
	    	tamanioBlocks++;*/
	int tamanioBlocks= config_valores.block_size * config_valores.blocks;

	return tamanioBlocks;
}


