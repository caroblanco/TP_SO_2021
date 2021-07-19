#include "gestion_bloques.h"


void agregarCaracter(char caracter, int cantidad){ //TODO VER SI HAY QUE GARANTIZAR MUTUA EXCLUSION DE PERSONAS TRABAJANDO SOBRE ESTOS ARCHIVOS
	int bloquesDelSist= config_valores.blocks;
	if(caracter=='O') {

        t_config* oxigeno_file = config_create(metadata_file_path_oxigeno);
        //TENDRIA QUE HABER UN MUTEX


        int cantidadDeOEscritas =  config_get_int_value(oxigeno_file, "SIZE");

         
        int cantBloques = config_get_int_value(oxigeno_file, "BLOCK_COUNT");
        char** bloquesUsados = config_get_array_value(oxigeno_file, "BLOCKS");



        int tamanioBloque = config_valores.block_size;

        //TENDRIA QUE HABER UN MUTEX

        int cantEscrita=0;
        int bloqueAUsar;
        char* actualizarBloquesUsador = string_new();
        int cantBloquesActualiz=cantBloques;
        //agarrar ultimo bloque usado, si no hay ninguno, crear un bloque
        if(cantBloques == 0) {
            //pedir un nuevo bloque (bitarray_test_bit(bitarray, i) == 0)
            for(int i=0; i<bloquesDelSist; i++) {
            	pthread_mutex_lock(&sincro_bitmap); //TODO LA SECCION CRITICA PODRIA O DEBERIA SER MAS CORTA
            										//PERO POR AHORA ES MAS FACIL ASI
                if((bitarray_test_bit(bitmap, i) == 0) && cantidad>0) {
                    bloqueAUsar = i;
                    cantBloquesActualiz ++;
                    char* bloqueNuevo= string_itoa(i);
                    cantEscrita=0;

                    //string_append(&actualizarBloquesUsador, "[");
                    if(cantBloquesActualiz==1)
                    string_append(&actualizarBloquesUsador, bloqueNuevo);
                    else{
                    string_append(&actualizarBloquesUsador, ",");
                    string_append(&actualizarBloquesUsador, bloqueNuevo);
                    }
                    free(bloqueNuevo);
                    // TODO FALTA ACTUALIZAR MD5 Y SIZE, PERO COMO NO SE CALCULARLOS SE DEJA PA MAS ADELANTE

                    bitarray_set_bit(bitmap,i);
                    msync(bitmap->bitarray, tamanoBitmap, MS_SYNC);
                    while(cantidad>0 && cantEscrita<tamanioBloque){
                    	pthread_mutex_lock(&sincro_block);
                    	memcpy(copiaBlock + bloqueAUsar * tamanioBloque + (cantidadDeOEscritas % tamanioBloque), "O", sizeof(char));
                    	pthread_mutex_unlock(&sincro_block);
                    	cantEscrita++;
                    	cantidad--;
                    	cantidadDeOEscritas++;

                    }
                    if(cantidad<=0){
                    	i=bloquesDelSist;
                    }
                    }
                pthread_mutex_unlock(&sincro_bitmap);
                }
            } else {
                bloqueAUsar = atoi(bloquesUsados[cantBloques-1]);
                for(int j=0; j<cantidad; j++) {
                    if(cantidadDeOEscritas % tamanioBloque == 0) {
                        for(int i=0; i<bloquesDelSist; i++) {
                        	pthread_mutex_lock(&sincro_bitmap);
                            if(bitarray_test_bit(bitmap, i) == 0) {
                                bloqueAUsar = i;
                                char* bloqueNuevo= string_itoa(i);
                                string_append(&actualizarBloquesUsador, ",");
                                string_append(&actualizarBloquesUsador, bloqueNuevo);
                                cantBloquesActualiz ++;
                                bitarray_set_bit(bitmap,i);
                                msync(bitmap->bitarray, tamanoBitmap, MS_SYNC);
                                i=bloquesDelSist;
                                free(bloqueNuevo);
                            }
                            pthread_mutex_unlock(&sincro_bitmap);
                        }

                    }
                    pthread_mutex_lock(&sincro_block);
                    memcpy(copiaBlock + bloqueAUsar * tamanioBloque + (cantidadDeOEscritas % tamanioBloque), "O", sizeof(char));
                    pthread_mutex_unlock(&sincro_block);
                    cantidadDeOEscritas++;
                }
            }
		char* nuevoMd5 =calcularMd5(cantidadDeOEscritas,'O'); 
		char* actualizarCant= string_itoa(cantBloquesActualiz);
        char* actualizarBloques  = string_new();
        char* actualizarSize= string_itoa(cantidadDeOEscritas);
        string_append(&actualizarBloques,"[");
        for(int i=0; i<cantBloques; i++) {
        	if(i==0)
        	string_append(&actualizarBloques,bloquesUsados[i]);
        	else{
        		string_append(&actualizarBloques,",");
        		string_append(&actualizarBloques,bloquesUsados[i]);
        	}

        }
        string_append(&actualizarBloques,actualizarBloquesUsador);
        string_append(&actualizarBloques,"]");
        config_set_value(oxigeno_file,"BLOCKS",actualizarBloques);
        config_set_value(oxigeno_file,"BLOCK_COUNT",actualizarCant);
        config_set_value(oxigeno_file,"SIZE",actualizarSize);
		config_set_value(oxigeno_file,"MD5_ARCHIVO",nuevoMd5);
        // TODO FALTA ACTUALIZAR MD5 Y SIZE, PERO COMO NO SE CALCULARLOS SE DEJA PA MAS ADELANTE
        config_save(oxigeno_file);
        config_destroy(oxigeno_file);
        free(nuevoMd5);
        free(bloquesUsados);
        free(actualizarBloques);
        free(actualizarBloquesUsador); //TODO LEAK
        free(actualizarCant);//TODO LEAK
        free(actualizarSize);
    } else if(caracter=='B') {
    	 t_config* basura_file = config_create(metadata_file_path_basura);
    	        //TENDRIA QUE HABER UN MUTEX


    	        int cantidadDeOEscritas =  config_get_int_value(basura_file, "SIZE");

    	        //BLOQUESUSADOS DEBERIA TENER UN TAMANIO FIJO Y SOLO DEBERIA IMPORTARME LOS PRIMEROS (CANTBLOQUES-1)
    	        int cantBloques = config_get_int_value(basura_file, "BLOCK_COUNT");
    	        char** bloquesUsados = config_get_array_value(basura_file, "BLOCKS");

    	        //int blocks = copiaBlock;

    	        int tamanioBloque = config_valores.block_size;

    	        //TENDRIA QUE HABER UN MUTEX

    	        int cantEscrita=0;
    	        int bloqueAUsar;
    	        char* actualizarBloquesUsador = string_new();
    	        int cantBloquesActualiz=cantBloques;
    	        //agarrar ultimo bloque usado, si no hay ninguno, crear un bloque
    	        if(cantBloques == 0) {
    	            //pedir un nuevo bloque (bitarray_test_bit(bitarray, i) == 0)
    	            for(int i=0; i<bloquesDelSist; i++) {
    	            	pthread_mutex_lock(&sincro_bitmap);
    	                if((bitarray_test_bit(bitmap, i) == 0) && cantidad>0) {
    	                    bloqueAUsar = i;
    	                    cantBloquesActualiz ++;
    	                    char* bloqueNuevo= string_itoa(i);
    	                    cantEscrita=0;

    	                    //string_append(&actualizarBloquesUsador, "[");
    	                    if(cantBloquesActualiz==1)
    	                    string_append(&actualizarBloquesUsador, bloqueNuevo);
    	                    else{
    	                    string_append(&actualizarBloquesUsador, ",");
    	                    string_append(&actualizarBloquesUsador, bloqueNuevo);
    	                    free(bloqueNuevo);
    	                    }
    	                    // TODO FALTA ACTUALIZAR MD5 Y SIZE, PERO COMO NO SE CALCULARLOS SE DEJA PA MAS ADELANTE

    	                    bitarray_set_bit(bitmap,i);
    	                    msync(bitmap->bitarray, tamanoBitmap, MS_SYNC);
    	                    while(cantidad>0 && cantEscrita<tamanioBloque){
    	                    	pthread_mutex_lock(&sincro_block);
    	                    	memcpy(copiaBlock + bloqueAUsar * tamanioBloque + (cantidadDeOEscritas % tamanioBloque), "B", sizeof(char));
    	                    	pthread_mutex_unlock(&sincro_block);
    	                    	cantEscrita++;
    	                    	cantidad--;
    	                    	cantidadDeOEscritas++;

    	                    }
    	                    if(cantidad<=0){
    	                       i=bloquesDelSist;
    	                     }
    	                    }
    	                pthread_mutex_unlock(&sincro_bitmap);
    	                }
    	            } else {
    	                bloqueAUsar = atoi(bloquesUsados[cantBloques-1]);
    	                for(int j=0; j<cantidad; j++) {
    	                    if(cantidadDeOEscritas % tamanioBloque == 0) {
    	                        for(int i=0; i<bloquesDelSist; i++) {
    	                        	pthread_mutex_lock(&sincro_bitmap);
    	                            if(bitarray_test_bit(bitmap, i) == 0) {
    	                                bloqueAUsar = i;
    	                                char* bloqueNuevo= string_itoa(i);
    	                                string_append(&actualizarBloquesUsador, ",");
    	                                string_append(&actualizarBloquesUsador, bloqueNuevo);
    	                                cantBloquesActualiz ++;
    	                                bitarray_set_bit(bitmap,i);
    	                                msync(bitmap->bitarray, tamanoBitmap, MS_SYNC);
    	                                i=bloquesDelSist;
    	                                free(bloqueNuevo);
    	                            }
    	                            pthread_mutex_unlock(&sincro_bitmap);
    	                        }
    	                    }
    	                    pthread_mutex_lock(&sincro_block);
    	                    memcpy(copiaBlock + bloqueAUsar * tamanioBloque + (cantidadDeOEscritas % tamanioBloque), "B", sizeof(char));
    	                    pthread_mutex_unlock(&sincro_block);
    	                    cantidadDeOEscritas++;
    	             
				   }
    	            }
    	        char* nuevoMd5 =calcularMd5(cantidadDeOEscritas,'B');
    	        char* actualizarCant= string_itoa(cantBloquesActualiz);
    	        char* actualizarBloques  = string_new();
    	        char* actualizarSize= string_itoa(cantidadDeOEscritas);
    	        string_append(&actualizarBloques,"[");
    	        for(int i=0; i<cantBloques; i++) {
    	        	if(i==0)
    	        	string_append(&actualizarBloques,bloquesUsados[i]);
    	        	else{
    	        		string_append(&actualizarBloques,",");
    	        		string_append(&actualizarBloques,bloquesUsados[i]);
    	        	}

    	        }
    	        string_append(&actualizarBloques,actualizarBloquesUsador);
    	        string_append(&actualizarBloques,"]");
    	        config_set_value(basura_file,"BLOCKS",actualizarBloques);
    	        config_set_value(basura_file,"BLOCK_COUNT",actualizarCant);
    	        config_set_value(basura_file,"SIZE",actualizarSize);
				config_set_value(basura_file,"MD5_ARCHIVO",nuevoMd5);
    	        // TODO FALTA ACTUALIZAR MD5 Y SIZE, PERO COMO NO SE CALCULARLOS SE DEJA PA MAS ADELANTE
    	        config_save(basura_file);
    	        config_destroy(basura_file);
    	        free(nuevoMd5);
    	        free(bloquesUsados);
    	        free(actualizarSize);
    	        free(actualizarCant);
    	        free(actualizarBloques);
    	        free(actualizarBloquesUsador);
    } else {
    	t_config* comida_file = config_create(metadata_file_path_comida);
    	    	        //TENDRIA QUE HABER UN MUTEX


    	    	        int cantidadDeOEscritas =  config_get_int_value(comida_file, "SIZE");

    	    	        //BLOQUESUSADOS DEBERIA TENER UN TAMANIO FIJO Y SOLO DEBERIA IMPORTARME LOS PRIMEROS (CANTBLOQUES-1)
    	    	        int cantBloques = config_get_int_value(comida_file, "BLOCK_COUNT");
    	    	        char** bloquesUsados = config_get_array_value(comida_file, "BLOCKS");



    	    	        int tamanioBloque = config_valores.block_size;

    	    	        //TENDRIA QUE HABER UN MUTEX

    	    	        int cantEscrita=0;
    	    	        int bloqueAUsar;
    	    	        char* actualizarBloquesUsador = string_new();
    	    	        int cantBloquesActualiz=cantBloques;
    	    	        //agarrar ultimo bloque usado, si no hay ninguno, crear un bloque
    	    	        if(cantBloques == 0) {
    	    	            //pedir un nuevo bloque (bitarray_test_bit(bitarray, i) == 0)
    	    	            for(int i=0; i<bloquesDelSist; i++) {
    	    	            	pthread_mutex_lock(&sincro_bitmap);
    	    	                if((bitarray_test_bit(bitmap, i) == 0) && cantidad>0) {
    	    	                    bloqueAUsar = i;
    	    	                    cantBloquesActualiz ++;
    	    	                    char* bloqueNuevo= string_itoa(i);
    	    	                    cantEscrita=0;

    	    	                    //string_append(&actualizarBloquesUsador, "[");
    	    	                    if(cantBloquesActualiz==1)
    	    	                    string_append(&actualizarBloquesUsador, bloqueNuevo);
    	    	                    else{
    	    	                    string_append(&actualizarBloquesUsador, ",");
    	    	                    string_append(&actualizarBloquesUsador, bloqueNuevo);
    	    	                    }
    	    	                    // TODO FALTA ACTUALIZAR MD5 Y SIZE, PERO COMO NO SE CALCULARLOS SE DEJA PA MAS ADELANTE
    	    	                    free(bloqueNuevo);
    	    	                    bitarray_set_bit(bitmap,i);
    	    	                    msync(bitmap->bitarray, tamanoBitmap, MS_SYNC);
    	    	                    while(cantidad>0 && cantEscrita<tamanioBloque){
    	    	                    	pthread_mutex_lock(&sincro_block);
    	    	                    	memcpy(copiaBlock + bloqueAUsar * tamanioBloque + (cantidadDeOEscritas % tamanioBloque), "C", sizeof(char));
    	    	                    	pthread_mutex_unlock(&sincro_block);
    	    	                    	cantEscrita++;
    	    	                    	cantidad--;
    	    	                    	cantidadDeOEscritas++;

    	    	                    }
    	    	                    if(cantidad<=0){
    	    	                       i=bloquesDelSist;
    	    	                     }
    	    	                    }
    	    	                pthread_mutex_unlock(&sincro_bitmap);
    	    	                }
    	    	            } else {
    	    	                bloqueAUsar = atoi(bloquesUsados[cantBloques-1]);
    	    	                for(int j=0; j<cantidad; j++) {
    	    	                    if(cantidadDeOEscritas % tamanioBloque == 0) {
    	    	                        for(int i=0; i<bloquesDelSist; i++) {
    	    	                        	pthread_mutex_lock(&sincro_bitmap);
    	    	                            if(bitarray_test_bit(bitmap, i) == 0) {
    	    	                                bloqueAUsar = i;
    	    	                                char* bloqueNuevo= string_itoa(i);
    	    	                                string_append(&actualizarBloquesUsador, ",");
    	    	                                string_append(&actualizarBloquesUsador, bloqueNuevo);
    	    	                                free(bloqueNuevo);
    	    	                                cantBloquesActualiz ++;
    	    	                                bitarray_set_bit(bitmap,i);
    	    	                                msync(bitmap->bitarray, tamanoBitmap, MS_SYNC);
    	    	                                i=bloquesDelSist;
    	    	                            }
    	    	                            pthread_mutex_unlock(&sincro_bitmap);
    	    	                        }
    	    	                    }
    	    	                    pthread_mutex_lock(&sincro_block);
    	    	                    memcpy(copiaBlock + bloqueAUsar * tamanioBloque + (cantidadDeOEscritas % tamanioBloque), "C", sizeof(char));
    	    	                    pthread_mutex_unlock(&sincro_block);
    	    	                    cantidadDeOEscritas++;
    	    	             
						  }
    	    	            }
    	    	        char* actualizarCant= string_itoa(cantBloquesActualiz);
    	    	        char* actualizarBloques  = string_new();
    	    	        char* actualizarSize= string_itoa(cantidadDeOEscritas);
    	    	        string_append(&actualizarBloques,"[");
    	    	        for(int i=0; i<cantBloques; i++) {
    	    	        	if(i==0)
    	    	        	string_append(&actualizarBloques,bloquesUsados[i]);
    	    	        	else{
    	    	        		string_append(&actualizarBloques,",");
    	    	        		string_append(&actualizarBloques,bloquesUsados[i]);
    	    	        	}

    	    	        }
    	    	        char* nuevoMd5 =calcularMd5(cantidadDeOEscritas,'C');
    	    	        string_append(&actualizarBloques,actualizarBloquesUsador);
    	    	        string_append(&actualizarBloques,"]");
    	    	        config_set_value(comida_file,"BLOCKS",actualizarBloques);
    	    	        config_set_value(comida_file,"BLOCK_COUNT", actualizarCant);
						config_set_value(comida_file,"MD5_ARCHIVO",nuevoMd5);
    	    	        config_set_value(comida_file,"SIZE",actualizarSize);
    	    	        // TODO FALTA ACTUALIZAR MD5 Y SIZE, PERO COMO NO SE CALCULARLOS SE DEJA PA MAS ADELANTE
    	    	        config_save(comida_file);
    	    	        config_destroy(comida_file);
    	    	        free(nuevoMd5);
    	    	        free(actualizarCant);
    	    	        free(actualizarBloques);
    	    	        free(actualizarSize);
    	    	        free(bloquesUsados);
    	    	        free(actualizarBloquesUsador);
    }

}

void borrarCaracter(char caracter, int cantidad){
    int tamanioBloque = config_valores.block_size;
    if(caracter=='O') {
    	int tamanioBloque = config_valores.block_size;
    	t_config* oxigeno_file = config_create(metadata_file_path_oxigeno);
    	        //TENDRIA QUE HABER UN MUTEX


    	        int cantidadDeOEscritas =  config_get_int_value(oxigeno_file, "SIZE");

    	        //BLOQUESUSADOS DEBERIA TENER UN TAMANIO FIJO Y SOLO DEBERIA IMPORTARME LOS PRIMEROS (CANTBLOQUES-1)
    	        int cantBloques = config_get_int_value(oxigeno_file, "BLOCK_COUNT");
    	        char** bloquesUsados = config_get_array_value(oxigeno_file, "BLOCKS");


    		if(cantidadDeOEscritas>=cantidad){
    			char* nuevosBloques= string_new();
    			for(int i=0; i<cantidad; i++) {

    				cantidadDeOEscritas--;
    				pthread_mutex_lock(&sincro_block);
        			memcpy(copiaBlock + atoi(bloquesUsados[cantBloques-1]) * tamanioBloque + (cantidadDeOEscritas % tamanioBloque), "W", sizeof(char));
        			pthread_mutex_unlock(&sincro_block);
    			   if((cantidadDeOEscritas%tamanioBloque) ==0){
    				   cantBloques--;
    				   int bitAcambiar= atoi(bloquesUsados[cantBloques]);
    				   bitarray_clean_bit(bitmap,bitAcambiar);

    			   }

    			}
    			string_append(&nuevosBloques,"[");
    			for(int i=0;i<cantBloques;i++){
    				if (i==0){
    					string_append(&nuevosBloques,bloquesUsados[i]);
    				}

    				else{
    					string_append(&nuevosBloques,",");
    					string_append(&nuevosBloques,bloquesUsados[i]);
				 
    			}
				 }
    			char* nuevoMd5 =calcularMd5(cantidadDeOEscritas,'O');
    			char* nuevaCant= string_itoa(cantBloques);
    			char* nuevaCantLetras= string_itoa(cantidadDeOEscritas);
    			string_append(&nuevosBloques,"]");
    			config_set_value(oxigeno_file,"BLOCKS",nuevosBloques);
    			config_set_value(oxigeno_file,"BLOCK_COUNT",nuevaCant);
    			config_set_value(oxigeno_file,"SIZE",nuevaCantLetras);
    			config_set_value(oxigeno_file,"MD5_ARCHIVO",nuevoMd5);
    			config_save(oxigeno_file);
    			free(nuevoMd5);
    			free(nuevaCant);
    			free(nuevaCantLetras);

    		}
    		else{
    			printf("No hay suficientes 'O' para borrar");
        			for(int i=0; i<cantidadDeOEscritas; i++) {

        				cantidadDeOEscritas--;
        				pthread_mutex_lock(&sincro_block);

            			memcpy(copiaBlock + atoi(bloquesUsados[cantBloques-1]) * tamanioBloque + (cantidadDeOEscritas % tamanioBloque), "W", sizeof(char));
            			pthread_mutex_unlock(&sincro_block);
        			   if((cantidadDeOEscritas%tamanioBloque) ==0){
        				   cantBloques--;

        			   }

        			}
    			for(int i=0;i<cantBloques;i++){
    				int bitAcambiar= atoi(bloquesUsados[i]);
    			    bitarray_clean_bit(bitmap,bitAcambiar);
    			}
    			cantBloques=0;
    			char* nuevoMd5 =calcularMd5(0,'O');
    			config_set_value(oxigeno_file,"BLOCKS","[]");
    			config_set_value(oxigeno_file,"BLOCK_COUNT","0");
    			config_set_value(oxigeno_file,"SIZE","0");
    			config_set_value(oxigeno_file,"MD5_ARCHIVO",nuevoMd5);
    			config_save(oxigeno_file);
    			free(nuevoMd5);
    		}
    		free(bloquesUsados);
    		config_destroy(oxigeno_file);
    } else if(caracter=='B') {

    	t_config* basura_file = config_create(metadata_file_path_basura);
    	char* nuevoMd5 =calcularMd5(0,'B');
    	int cantBloques = config_get_int_value(basura_file, "BLOCK_COUNT");
        char** bloquesUsados = config_get_array_value(basura_file, "BLOCKS");
        int cantidadDeBEscritas =  config_get_int_value(basura_file, "SIZE");


		for(int i=0; i<cantidadDeBEscritas; i++) {

			cantidadDeBEscritas--;
			pthread_mutex_lock(&sincro_block);
			memcpy(copiaBlock + atoi(bloquesUsados[cantBloques-1]) * tamanioBloque + (cantidadDeBEscritas % tamanioBloque), "W", sizeof(char));
			pthread_mutex_unlock(&sincro_block);
		   if((cantidadDeBEscritas%tamanioBloque) ==0){
			   cantBloques--;

		   }

		}
        for(int i=0;i<cantBloques;i++){
             int bitAcambiar= atoi(bloquesUsados[i]);
             bitarray_clean_bit(bitmap,bitAcambiar);
            			}
        				config_set_value(basura_file,"MD5_ARCHIVO",nuevoMd5);
    	    			config_set_value(basura_file,"BLOCKS","[]");
    	    			config_set_value(basura_file,"BLOCK_COUNT","0");
    	    			config_set_value(basura_file,"SIZE","0");
    	    			config_save(basura_file);
    	    			free(nuevoMd5);
    	    			free(bloquesUsados);
    	    			config_destroy(basura_file);
    } else {
    	int tamanioBloque = config_valores.block_size;
    	    	t_config* comida_file = config_create(metadata_file_path_comida);
    	    	        //TENDRIA QUE HABER UN MUTEX


    	    	        int cantidadDeCEscritas =  config_get_int_value(comida_file, "SIZE");

    	    	        //BLOQUESUSADOS DEBERIA TENER UN TAMANIO FIJO Y SOLO DEBERIA IMPORTARME LOS PRIMEROS (CANTBLOQUES-1)
    	    	        int cantBloques = config_get_int_value(comida_file, "BLOCK_COUNT");
    	    	        char** bloquesUsados = config_get_array_value(comida_file, "BLOCKS");


    	    		if(cantidadDeCEscritas>=cantidad){
    	    			char* nuevosBloques= string_new();
    	    			for(int i=0; i<cantidad; i++) {
    	    				cantidadDeCEscritas--;
    	    				pthread_mutex_lock(&sincro_block);
    	        			memcpy(copiaBlock + atoi(bloquesUsados[cantBloques-1]) * tamanioBloque + (cantidadDeCEscritas % tamanioBloque), "W", sizeof(char));
    	        			pthread_mutex_unlock(&sincro_block);
    	    			   if(cantidadDeCEscritas%tamanioBloque ==0){
    	    				   cantBloques--;
    	    				   int bitAcambiar= atoi(bloquesUsados[cantBloques]);
    	    				   bitarray_clean_bit(bitmap,bitAcambiar);
    	    			   }

    	    			}
    	    			string_append(&nuevosBloques,"[");
    	    			for(int i=0;i<cantBloques;i++){
    	    				if (i==0){
    	    					string_append(&nuevosBloques,bloquesUsados[i]);
    	    				}

    	    				else{
    	    					string_append(&nuevosBloques,",");
    	    					string_append(&nuevosBloques,bloquesUsados[i]);
    	    				}
    	    			}
    	    			char* nuevoMd5 =calcularMd5(cantidadDeCEscritas,'C');
    	    			char* nuevaCant= string_itoa(cantBloques);
    	    			char* nuevaCantLetras= string_itoa(cantidadDeCEscritas);
    	    			string_append(&nuevosBloques,"]");
    	    			config_set_value(comida_file,"MD5_ARCHIVO",nuevoMd5);
    	    			config_set_value(comida_file,"BLOCKS",nuevosBloques);
    	    			config_set_value(comida_file,"BLOCK_COUNT",nuevaCant);
    	    			config_set_value(comida_file,"SIZE",nuevaCantLetras);
    	    			config_save(comida_file);
    	    			free(nuevoMd5);
    	    			free(nuevaCant);
    	    			free(nuevaCantLetras);
    	    		}
    	    		else{
    	    			printf("No hay suficientes 'C' para borrar");
    	    			for(int i=0; i<cantidadDeCEscritas; i++) {

    	    				cantidadDeCEscritas--;
    	    				pthread_mutex_lock(&sincro_block);
    	    				memcpy(copiaBlock + atoi(bloquesUsados[cantBloques-1]) * tamanioBloque + (cantidadDeCEscritas % tamanioBloque), "W", sizeof(char));
    	    				pthread_mutex_unlock(&sincro_block);
    	    			   if((cantidadDeCEscritas%tamanioBloque) ==0){
    	    				   cantBloques--;

    	    			   }

    	    			}
    	    			for(int i=0;i<cantBloques;i++){
    	    			   int bitAcambiar= atoi(bloquesUsados[i]);
    	    			   bitarray_clean_bit(bitmap,bitAcambiar);
    	    			    			}
    	    			cantBloques=0;
    	    			char* nuevoMd5 =calcularMd5(0,'C');
    	    			config_set_value(comida_file,"MD5_ARCHIVO",nuevoMd5);
    	    			config_set_value(comida_file,"BLOCKS","[]");
    	    			config_set_value(comida_file,"BLOCK_COUNT","0");
    	    			config_set_value(comida_file,"SIZE","0");
    	    			config_save(comida_file);
    	    			free(nuevoMd5);
    	    		}

    	    		free(bloquesUsados);
    	    	    config_destroy(comida_file);
    }

}

void actualizarUbicacionBitacora(struct_ubicacion* struct_ub){//int id,int x_nueva,int y_nueva,int x_vieja,int y_vieja){
	int bloquesDelSist= config_valores.blocks;
	int tamanioBloque = config_valores.block_size;
	char* bitacora_tripu = string_new();
	char* idTripu= string_itoa(struct_ub->id);
	char* x1=string_itoa(struct_ub->x_vieja);
	char* y1=string_itoa(struct_ub->y_vieja);
	char* x2=string_itoa(struct_ub->x_nueva);
	char* y2=string_itoa(struct_ub->y_nueva);
	string_append(&bitacora_tripu, bitacora_path);
	string_append(&bitacora_tripu, "Tripulante");
	string_append(&bitacora_tripu, idTripu);
	string_append(&bitacora_tripu, ".ims");
	//pthread_mutex_lock(&bitacora);
	t_config* bitacora_config = config_create(bitacora_tripu);
	if (bitacora_config == NULL) {
		perror("Bitacora de tripulante  no encontrada");
		return;
	}
	int cantidadDeCaracteres =  config_get_int_value(bitacora_config, "SIZE");
    char** bloquesUsados = config_get_array_value(bitacora_config, "BLOCKS");
	int cantBloques = -1;


    while (bloquesUsados[++cantBloques] != NULL){}

	char* presentacion= string_new();
	string_append(&presentacion,"Se mueve de "); // tam 11
	string_append(&presentacion,x1);
	string_append(&presentacion,"|");
	string_append(&presentacion,y1);
	string_append(&presentacion," a ");
	string_append(&presentacion,x2);
	string_append(&presentacion,"|");
	string_append(&presentacion,y2);
	int letras=string_length(presentacion);
	int bloqueAUsar=0;
	int	cantBloquesActualiz=cantBloques;
	int cantEscrita=0;
	int posicionQueToca=0;
	char* actualizarBloquesUsador = string_new();
	if(cantBloques == 0) {
	            //pedir un nuevo bloque (bitarray_test_bit(bitarray, i) == 0)
	            for(int i=0; i<bloquesDelSist; i++) {
	            	pthread_mutex_lock(&sincro_bitmap);
	                if((bitarray_test_bit(bitmap, i) == 0) && letras>0) {
	                    bloqueAUsar = i;
	                    cantBloquesActualiz ++;
	                    char* bloqueNuevo= string_itoa(i);
	                    cantEscrita=0;

	                    //string_append(&actualizarBloquesUsador, "[");
	                    if(cantBloquesActualiz==1)
	                    string_append(&actualizarBloquesUsador, bloqueNuevo);
	                    else{
	                    string_append(&actualizarBloquesUsador, ",");
	                    string_append(&actualizarBloquesUsador, bloqueNuevo);
	                    free(bloqueNuevo);
	                    }
	                    // TODO FALTA ACTUALIZAR MD5 Y SIZE, PERO COMO NO SE CALCULARLOS SE DEJA PA MAS ADELANTE

	                    bitarray_set_bit(bitmap,i);
	                    msync(bitmap->bitarray, tamanoBitmap, MS_SYNC);
	                    while(letras>0 && cantEscrita<tamanioBloque){
	                    	char* caracterAgregar= malloc(2);
	                    	caracterAgregar[0]= presentacion[posicionQueToca];
	                    	caracterAgregar[1]='\0';
	                    	pthread_mutex_lock(&sincro_block);
	                    	memcpy(copiaBlock + bloqueAUsar * tamanioBloque + (cantidadDeCaracteres % tamanioBloque), caracterAgregar, sizeof(char));
	                    	pthread_mutex_unlock(&sincro_block);
	                    	cantEscrita++;
	                    	letras--;
	                    	cantidadDeCaracteres++;
	                    	posicionQueToca++;
	                    	free(caracterAgregar);
	                    	if(cantEscrita<tamanioBloque){
		                    	pthread_mutex_lock(&sincro_block);
		                    	memcpy(copiaBlock + bloqueAUsar * tamanioBloque + (cantidadDeCaracteres % tamanioBloque), "\0", sizeof(char));
		                    	pthread_mutex_unlock(&sincro_block);
	                    	}
	                    }
	                    }
	               pthread_mutex_unlock(&sincro_bitmap);
	               if(letras==0){
	            	   i=bloquesDelSist;
	               }
	                }
	            } else {
	                bloqueAUsar = atoi(bloquesUsados[cantBloques-1]);
	                for(int j=0; j<letras; j++) {
	                    if(cantidadDeCaracteres % tamanioBloque == 0) {
	                        for(int i=0; i<bloquesDelSist; i++) {
	                        	pthread_mutex_lock(&sincro_bitmap);
	                            if(bitarray_test_bit(bitmap, i) == 0) {
	                                bloqueAUsar = i;
	                                char* bloqueNuevo= string_itoa(i);
	                                string_append(&actualizarBloquesUsador, ",");
	                                string_append(&actualizarBloquesUsador, bloqueNuevo);
	                                cantBloquesActualiz ++;
	                                free(bloqueNuevo);
	                                bitarray_set_bit(bitmap,i);
	                                msync(bitmap->bitarray, tamanoBitmap, MS_SYNC);
	                                i=bloquesDelSist;
	                            }
	                            pthread_mutex_unlock(&sincro_bitmap);
	                        }
	                    }
	                    char* caracterAgregar= malloc(2);
	                    caracterAgregar[0]= presentacion[posicionQueToca];
	                    caracterAgregar[1]='\0';
	                    pthread_mutex_lock(&sincro_block);
	                    memcpy(copiaBlock + bloqueAUsar * tamanioBloque + (cantidadDeCaracteres % tamanioBloque), caracterAgregar, sizeof(char));
	                    pthread_mutex_unlock(&sincro_block);
	                    cantidadDeCaracteres++;
	                    posicionQueToca++;
	                    free(caracterAgregar);
	                    if(cantidadDeCaracteres % tamanioBloque != 0){
	                    pthread_mutex_lock(&sincro_block);
	                    memcpy(copiaBlock + bloqueAUsar * tamanioBloque + (cantidadDeCaracteres % tamanioBloque), "\0", sizeof(char));
	                    pthread_mutex_unlock(&sincro_block);
	                    }

	                }
	            }
	        char* actualizarBloques  = string_new();
	        char* actualizarSize=string_new();
		string_append(&actualizarSize,string_itoa(cantidadDeCaracteres) );
	        string_append(&actualizarBloques,"[");
	        for(int i=0; i<cantBloques; i++) {
	        	if(i==0)
	        	string_append(&actualizarBloques,bloquesUsados[i]);
	        	else{
	        		string_append(&actualizarBloques,",");
	        		string_append(&actualizarBloques,bloquesUsados[i]);
	        	}

	        }
	        pthread_mutex_lock(&bitacora);
	        string_append(&actualizarBloques,"\0");
	        string_append(&actualizarBloquesUsador, "\0");
	        string_append(&actualizarSize,"\0");
	        string_append(&actualizarBloques,actualizarBloquesUsador);
	        string_append(&actualizarBloques,"]");
	        config_set_value(bitacora_config,"BLOCKS",actualizarBloques);
	        config_set_value(bitacora_config,"SIZE",actualizarSize);
	        config_save(bitacora_config);
	        config_destroy(bitacora_config);
	       pthread_mutex_unlock(&bitacora);
	        free(bitacora_tripu);
	        free(idTripu);
	        free(x1);
	        free(y1);
	        free(x2);
	        free(y2);
	        liberarStringArray(bloquesUsados);
	        free(actualizarBloques);
	        free(actualizarSize);
	        free(actualizarBloquesUsador);
	        free(presentacion);
	        return;
	        
}

void actualizarTarea(int id,char* tarea){
	int bloquesDelSist= config_valores.blocks;
		int tamanioBloque = config_valores.block_size;
		char* bitacora_tripu = string_new();
		char* idTripu= string_itoa(id);
		string_append(&bitacora_tripu, bitacora_path);
		string_append(&bitacora_tripu, "Tripulante");
		string_append(&bitacora_tripu, idTripu);
		string_append(&bitacora_tripu, ".ims");
		// pthread_mutex_lock(&bitacora);
		t_config* bitacora_config = config_create(bitacora_tripu);
		int cantidadDeCaracteres =  config_get_int_value(bitacora_config, "SIZE");
		char** bloquesUsados = config_get_array_value(bitacora_config, "BLOCKS");

		int cantBloques=-1;

	    while (bloquesUsados[++cantBloques] != NULL){}

		char* presentacion= string_new();
		string_append(&presentacion,tarea);
		int letras=string_length(presentacion);
		int bloqueAUsar=0;
		int	cantBloquesActualiz=cantBloques;
		int cantEscrita=0;
		int posicionQueToca=0;
		char* actualizarBloquesUsador = string_new();
		if(cantBloques == 0) {
		            //pedir un nuevo bloque (bitarray_test_bit(bitarray, i) == 0)
		            for(int i=0; i<bloquesDelSist; i++) {
		            	pthread_mutex_lock(&sincro_bitmap);
		                if((bitarray_test_bit(bitmap, i) == 0) && letras>0) {
		                    bloqueAUsar = i;
		                    cantBloquesActualiz ++;
		                    char* bloqueNuevo= string_itoa(i);
		                    cantEscrita=0;

		                    //string_append(&actualizarBloquesUsador, "[");
		                    if(cantBloquesActualiz==1)

		                    string_append(&actualizarBloquesUsador, bloqueNuevo);
		                    else{
		                    string_append(&actualizarBloquesUsador, ",");
		                    string_append(&actualizarBloquesUsador, bloqueNuevo);
		                    free(bloqueNuevo);
		                    }
		                    // TODO FALTA ACTUALIZAR MD5 Y SIZE, PERO COMO NO SE CALCULARLOS SE DEJA PA MAS ADELANTE

		                    bitarray_set_bit(bitmap,i);
		                    msync(bitmap->bitarray, tamanoBitmap, MS_SYNC);
		                    while(letras>0 && cantEscrita<tamanioBloque){
		                    	char* caracterAgregar= malloc(2);
		                    	caracterAgregar[0]= presentacion[posicionQueToca];
		                    	caracterAgregar[1]='\0';
		                    	pthread_mutex_lock(&sincro_block);
		                    	memcpy(copiaBlock + bloqueAUsar * tamanioBloque + (cantidadDeCaracteres % tamanioBloque), caracterAgregar, sizeof(char));
		                    	pthread_mutex_unlock(&sincro_block);
		                    	cantEscrita++;
		                    	letras--;
		                    	cantidadDeCaracteres++;
		                    	posicionQueToca++;
		                    	free(caracterAgregar);
		                    	if(cantEscrita<tamanioBloque){
			                    	pthread_mutex_lock(&sincro_block);
			                    	memcpy(copiaBlock + bloqueAUsar * tamanioBloque + (cantidadDeCaracteres % tamanioBloque), "\0", sizeof(char));
			                    	pthread_mutex_unlock(&sincro_block);
		                    	}
		                    }
		                    if(letras==0){
		                    	i=bloquesDelSist;
		                    }
		                    }
		                pthread_mutex_unlock(&sincro_bitmap);
		                }
		            } else {
		                bloqueAUsar = atoi(bloquesUsados[cantBloques-1]);
		                for(int j=0; j<letras; j++) {

		                    if(cantidadDeCaracteres % tamanioBloque == 0) {
		                        for(int i=0; i<bloquesDelSist; i++) {
		                        	pthread_mutex_lock(&sincro_bitmap);
		                            if(bitarray_test_bit(bitmap, i) == 0) {
		                                bloqueAUsar = i;
		                                char* bloqueNuevo= string_itoa(i);
		                                string_append(&actualizarBloquesUsador, ",");
		                                string_append(&actualizarBloquesUsador, bloqueNuevo);
		                                free(bloqueNuevo);
		                                cantBloquesActualiz ++;
		                                bitarray_set_bit(bitmap,i);
		                                msync(bitmap->bitarray, tamanoBitmap, MS_SYNC);
		                                i=bloquesDelSist;
		                            }
		                            pthread_mutex_unlock(&sincro_bitmap);
		                        }

		                    }
		                    char* caracterAgregar= malloc(2);
		                    caracterAgregar[0]= presentacion[posicionQueToca];
		                    caracterAgregar[1]='\0';
		                    pthread_mutex_lock(&sincro_block);
		                    memcpy(copiaBlock + bloqueAUsar * tamanioBloque + (cantidadDeCaracteres % tamanioBloque), caracterAgregar, sizeof(char));
		                    pthread_mutex_unlock(&sincro_block);
		                    cantidadDeCaracteres++;
		                    posicionQueToca++;
		                    free(caracterAgregar);
		                    if(cantidadDeCaracteres % tamanioBloque != 0){
		                    pthread_mutex_lock(&sincro_block);
		                    memcpy(copiaBlock + bloqueAUsar * tamanioBloque + (cantidadDeCaracteres % tamanioBloque), "\0", sizeof(char));
		                    pthread_mutex_unlock(&sincro_block);
		                    }
		                }
		            }
		        char* actualizarBloques  = string_new();
		        char* actualizarSize=string_new();
			string_append(&actualizarSize,string_itoa(cantidadDeCaracteres) );
		        string_append(&actualizarBloques,"[");
		        for(int i=0; i<cantBloques; i++) {
		        	if(i==0)
		        	string_append(&actualizarBloques,bloquesUsados[i]);
		        	else{
		        		string_append(&actualizarBloques,",");
		        		string_append(&actualizarBloques,bloquesUsados[i]);
		        	}

		        }
		        pthread_mutex_lock(&bitacora);
		        string_append(&actualizarBloques,"\0");
		        string_append(&actualizarBloquesUsador, "\0");
		        string_append(&actualizarSize,"\0");
		        string_append(&actualizarBloques,actualizarBloquesUsador);
		        string_append(&actualizarBloques,"]");
		        config_set_value(bitacora_config,"BLOCKS",actualizarBloques);
		        config_set_value(bitacora_config,"SIZE",actualizarSize);
		        // TODO FALTA ACTUALIZAR MD5 Y SIZE, PERO COMO NO SE CALCULARLOS SE DEJA PA MAS ADELANTE
		        config_save(bitacora_config);

		        config_destroy(bitacora_config);
				pthread_mutex_unlock(&bitacora);
		        free(bloquesUsados);
		        free(idTripu);
		        free(actualizarBloquesUsador);
		        free(actualizarSize);
		        free(actualizarBloques);
		        free(presentacion);
		        free(bitacora_tripu);
		        return;
}

int buscarPrimerBloqueLibre(){
	int bloquesDelSist= config_valores.blocks;
	for(int i=0; i<bloquesDelSist; i++) {
	     if(bitarray_test_bit(bitmap, i) == 0){
	    	 bitarray_set_bit(bitmap,i);
	     	 msync(bitmap->bitarray, tamanoBitmap, MS_SYNC);
           return i;}
	}
	return EXIT_FAILURE; //QUE CARAJO RETORNO SI HAY ERROR?
}

int primerEspacioLibre(int bloqueAUtilizar, char caracter) {
    int i = 0, tamanioBloque = config_valores.block_size;
    char caracterEnBlock;
    memcpy(&caracterEnBlock,(copiaBlock + i + tamanioBloque * bloqueAUtilizar),1);
    while(i < tamanioBloque && caracterEnBlock == caracter) {
        i++;
        memcpy(&caracterEnBlock,(copiaBlock + i + tamanioBloque * bloqueAUtilizar),1);
    }
    if(i == tamanioBloque) {
        i=-1;
    }
    return i;
}

char* leerArchivo(char* nombreArch){

    FILE* file = fopen(nombreArch,"r");
    if(file == NULL)
    {
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long int size = 32;
    rewind(file);

    char* content = calloc(size + 1, 1);

    fread(content,1,size,file);

    return content;


}


char* calcularMd5(int tamanio, char caracter) {
   
    
    if(caracter=='O') {
        char* string= string_new();
        string_append(&string,"echo ");
        for(int i=0;i<tamanio;i++){
            string_append(&string,"O");
        }
        string_append(&string, " | md5sum > md5O") ;
    	int resultadoMd5 = system(string);
    	free(string);
        if(resultadoMd5>=0){
        	log_info(logger, "MD5 calculado correctamente");
           char* contenido= leerArchivo("md5O");
           return contenido ; 
        }
        else{
        	log_info(logger, "error al calcular MD5");
        }
    } else if(caracter=='C') {
                char* string= string_new();
        string_append(&string,"echo ");
        for(int i=0;i<tamanio;i++){
            string_append(&string,"C");
        }
        string_append(&string, " | md5sum > md5C") ;
    	int resultadoMd5 = system(string);
    	free(string);
        if(resultadoMd5>=0){
        	log_info(logger, "MD5 calculado correctamente");
            char* contenido= leerArchivo("md5C");
           return contenido ;
        }
        else{
        	log_info(logger, "error al calcular MD5");
        }
    } else {
        char* string= string_new();
        string_append(&string,"echo ");
        for(int i=0;i<tamanio;i++){
            string_append(&string,"B");
        }
        string_append(&string, " | md5sum > md5B") ;
    	int resultadoMd5 = system(string);
    	free(string);
        if(resultadoMd5>=0){
        	log_info(logger, "MD5 calculado correctamente");
            char* contenido= leerArchivo("md5B");
           return contenido ;
        }
        else{
        	log_info(logger, "error al calcular MD5");
        }
    }
    return "fail";
}




/*
int buscarPrimerBloqueLibre(){
	int bloquesDelSist= config_valores.blocks;
	for(int i=0; i<bloquesDelSist; i++) {
	     if(bitarray_test_bit(bitmap, i) == 0)
           return i;
	}
}


int escribirLoQueSePueda(int bloqueAUsar,char* escribir,int cantidad,int escritos){
	int i=0;
	int tamanioBloque = config_valores.block_size;
	while(escritos % tamanioBloque != 0 && i<=cantidad){
	pthread_mutex_lock(&sincro_block);
	memcpy(copiaBlock + bloqueAUsar * tamanioBloque + (escritos % tamanioBloque), escribir[i], sizeof(char));
	pthread_mutex_unlock(&sincro_block);
	escritos++;
	i++;
	}
	if(i<cantidad){
		int faltan= cantidad-i;
		return faltan;
	}
	return -1;
}*/

















