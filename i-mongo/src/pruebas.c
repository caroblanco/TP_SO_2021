/*void agregarCaracter(char caracter, int cantidad){
    if(caracter=='O') {
        //buscar archivo oxigeno.ims
        //int fd = open(metadata_file_path_oxigeno, O_RDWR, 0664);
        //void* archivo_oxigeno = mmap(NULL, config_valores.block_size * config_valores.blocks, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        t_config* oxigeno_file = config_create(metadata_file_path_oxigeno);
        //TENDRIA QUE HABER UN MUTEX


        int cantidadDeOEscritas =  config_get_int_value(metadata_file_path_oxigeno, "SIZE");

        //BLOQUESUSADOS DEBERIA TENER UN TAMANIO FIJO Y SOLO DEBERIA IMPORTARME LOS PRIMEROS (CANTBLOQUES-1)
        int cantBloques = config_get_int_value(metadata_file_path_oxigeno, "BLOCK_COUNT");
        char** bloquesUsados = config_get_array_value(metadata_file_path_oxigeno, "BLOCKS");

        char* bloqueDeLlenado = config_get_string_value(metadata_file_path_oxigeno, "CARACTER_LLENADO");
        char* md5Archivo = config_get_string_value(metadata_file_path_oxigeno, "MD5_ARCHIVO");
        int blocks = copiaBlock;

        int tamanioBloque = fs_superBloque.block_size;

        //TENDRIA QUE HABER UN MUTEX


    

        int bloqueAUsar;
    for(int j=0; j<cantidad; j++) {
        //agarrar ultimo bloque usado, si no hay ninguno, crear un bloque
        if(cantBloques == 0) {
            //pedir un nuevo bloque
            for(int i=0; i<fs_superBloque.blocks; i++) {
                if(fs_superBloque.bitmap[i] == 0) {
                    bloqueAUsar = i;
                    cantBloques ++;
                    char** bloquesUsados = malloc(1);
                    bloquesUsados[0] = itoa(i);
                    bitmap[i] = 1;
                    }
                }
            } else {
                bloqueAUsar = atoi(bloquesUsados[cantBloques]);
                
                    if(cantidadDeOEscritas % tamanioBloque == 0) {
                        for(int i=0; i<fs_superBloque.blocks; i++) {
                            if(fs_superBloque.bitmap[i] == 0) {
                                bloqueAUsar = i;
                                cantBloques ++;
                                char** auxBloquesUsados = malloc(cantBloques);
                                memcpy(auxBloquesUsados, bloquesUsados, cantBloques);
                                bloquesUsados = auxBloquesUsados;
                                auxBloquesUsados[cantBloques] = itoa(i);
                                bitmap[i] = 1;
                            }
                        }                   
                    }     
            }
        blocks[bloqueAUsar * tamanioBloque + cantidadDeOEscritas % tamanioBloque + 1] = "O";
        }
        
    }   


            //pedir un nuevo bloque
            for(int i=0; i<fs_superBloque.blocks; i++) {
                if(fs_superBloque.bitmap[i] == 0) {
                    bloqueAUsar = i;
                    cantBloques ++;
                    bloquesUsados [cantBloques] = i;
                    bitmap[i] = 1;
                }
            }
        } else {
            bloqueAUsar = bloquesUsados[cantBloques];
        }
        for(int i=0; i < cantidad; i++){
            if(primerEspacioLibre(bloqueAUsar) == -1) {
                for(int j=0; j <fs_superBloque.blocks; j++) {
                    if(bitmap[j] == 0) {
                        bloqueAUsar = j;
                        cantBloques ++;
                        bloquesUsados [cantBloques] = j;
                        bitmap[j] = 1;
                    }
                }
            }
            blocks[bloqueAUsar*fs_superBloque.block_size+primerEspacioLibre(bloqueAUsar)] = 'O';
        }
    } else if(caracter=='B') {
    } else {
    }
}*/





















/*

void hola() {



    t_config* comida_file = config_create(metadata_file_path_comida);

        int cantidadDeCEscritas =  config_get_int_value(comida_file, "SIZE");
        int cantBloquesCCC = config_get_int_value(comida_file, "BLOCK_COUNT");
        char** bloquesUsadosCCC = config_get_array_value(comida_file, "BLOCKS");

    // evaluar que los bloques en mi lista sean validos, ninguno negativo ni mayor al limite
    // arreglar los que esten mal y volver a escribir todas los caracteres
    

    

    if(sizeof(bloquesUsadosCCC) == cantBloquesCCC) {
        for (int i; i <= cantBloquesCCC; i++) {
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
        // reescribi TODAS LAS 'C'
        return;
    }

    if(sizeof(bloquesUsadosCCC) != cantBloquesCCC) {
        cantBloquesCCC = sizeof(bloquesUsadosCCC);
        char* actualizarCantCCC = string_itoa(cantBloquesCCC);
	    config_set_value(comida_file,"BLOCK_COUNT",actualizarCantCCC);
        return;
    }

    int cantidadDeBloquesLlenosCCC = cantBloquesCCC - 1;
    int cantidadDeCEnELUltimoBloque;
    int ultimoBloqueCCC = atoi(bloquesUsadosCCC[cantBloquesCCC]);

    if(primerEspacioLibre(ultimoBloqueCCC, 'C') == -1) {
        cantidadDeCEnELUltimoBloque = tamanioBlocks;
    } else {
        cantidadDeCEnELUltimoBloque = primerEspacioLibre(ultimoBloqueCCC, 'C') -1;
    }

    int tamanioPostaMedidoCCC = cantidadDeBloquesLlenosCCC + cantidadDeCEnELUltimoBloque;

    if(tamanioPostaMedidoCCC != cantidadDeCEscritas) {
        cantidadDeCEscritas = tamanioPostaMedidoCCC;
        // actualizar el valor en la metadata,  creo que se hace con set
        config_set_value(comida_file,"SIZE",cantidadDeCEscritas);
        return;
    }


}*/
