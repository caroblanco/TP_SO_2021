#include "init_file-system.h"


void iniciar_file_system() {
	//iniciar_log();
	log_info(logger, "Inicializando File system");
	crear_paths();

	crear_superBloque();
	pthread_t manejo_block;
			pthread_create(&manejo_block, NULL, (void*) crearYmanejar_blocks, NULL);
			pthread_detach(manejo_block);
	crear_metadata();
	log_info(logger, "File system inicializado");
}

void crear_paths() {
	log_info(logger, "Creando paths");
	verificar_dir(config_valores.punto_montaje);
	crear_metadata_path();                      //LA CARPETA DE LA METADATA SERA "FILES"
	//crear_superBloque_path();                   // DENTRO DEL PTO DE MONTAJE superbloque.ims
	crear_blocks_path();                       // DENTRO DEL PTO DE MONTAJE Blocks.ims
	crear_bitacora_path();
	//crear_bitmap_path();

	log_info(logger, "Paths creados correctamente");
	return;
}

void verificar_dir(char* path)
{
	if(!existe_dir(path))
	{
		printf("carpeta %s no encontrada, creando\n",path);
		mkdir(path, 0777); //mkdir tiene su origen en las palabras make subdirectory que significan crear subdirectorio en inglés.
	}					   // EL 0777 SIGNIFICA QUE TENGO CONTROL COMPLETO
	else
		printf("carpeta %s encontrada\n",path);

}

void crear_metadata_path() {
	metadata_path = string_new();
	string_append(&metadata_path, config_valores.punto_montaje);
	string_append(&metadata_path, "/Files/");
	verificar_dir(metadata_path);
}

void crear_blocks_path() {
	block_path = string_new();
	string_append(&block_path, config_valores.punto_montaje);
	string_append(&block_path, "/Blocks.ims");
	//verificar_dir(metadata_path); NO VA PARECE
}

void crear_bitacora_path() {
	bitacora_path = string_new();
	string_append(&bitacora_path, metadata_path);
	string_append(&bitacora_path, "Bitacoras/");
	verificar_dir(bitacora_path);
}

void crear_superBloque() {
	log_info(logger, "Buscando Filesystem existente");

	/*char**/ superBloque_file_path = string_new();
	string_append(&superBloque_file_path, config_valores.punto_montaje);
	string_append(&superBloque_file_path, "/SuperBloque.ims"); //CAMBIAR EL ARCHIVO

	int fd = open(superBloque_file_path, O_CREAT | O_RDWR, 0664);
	
	if(!existe_archivo(superBloque_file_path))
			printf("post superBloque no existe\n");
		else
			printf("post existe\n");
	if (fd == -1) {
			close(fd);
			log_error(logger, "Error abriendo el superBloque.ims");
			return EXIT_FAILURE;
		}
	double c = (double) config_valores.blocks;
	tamanoBitmap= (int) ceil( c/8.0 );
	ftruncate(fd, sizeof(int)*2 + tamanoBitmap );
	void* superBloque = mmap(NULL, sizeof(int)*2 + tamanoBitmap, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	
	if (superBloque == MAP_FAILED) {
			log_error(logger, "Error ejecutando el mmap(...)");
			close(fd);
			return EXIT_FAILURE;
		}
	void* uints= malloc(4);
	uints= realloc(uints, sizeof(int));
		memcpy(uints, &config_valores.block_size, sizeof(int));
		memcpy(superBloque, uints, sizeof(int));
		memcpy(uints, &config_valores.blocks, sizeof(int));
		memcpy(superBloque+sizeof(int), uints, sizeof(int));
		bitmap = bitarray_create_with_mode((char*) superBloque+sizeof(int)+sizeof(int), tamanoBitmap, MSB_FIRST); 
		msync(bitmap->bitarray, tamanoBitmap, MS_SYNC);
		msync(superBloque, sizeof(int)*2 + tamanoBitmap, MS_SYNC);
		close(fd);
		free(uints);
	return;
}

void crearYmanejar_blocks(){
	//char* addr;
	copiaBlock= malloc(config_valores.block_size * config_valores.blocks);
		if(!existe_archivo(block_path))
			printf("pre no existe\n");
		else
			printf("pre ya existe\n");
		int fd = open(block_path, O_CREAT | O_RDWR, 0664);

		if(!existe_archivo(block_path))
			printf("post no existe\n");
		else
			printf("post existe\n");

		if (fd == -1) {
			close(fd);
			log_error(logger, "Error abriendo el Blocks.ims");
			return EXIT_FAILURE;
		}

		ftruncate(fd, config_valores.block_size * config_valores.blocks);

		void* block = mmap(NULL, config_valores.block_size * config_valores.blocks, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

		if (block == MAP_FAILED) {
			log_error(logger, "Error ejecutando el mmap(...)");
			close(fd);
			return EXIT_FAILURE;
		}
		//printf("%s",block);
		pthread_mutex_lock(&sincro_block);
		memcpy(copiaBlock,block,config_valores.block_size * config_valores.blocks);
		pthread_mutex_unlock(&sincro_block);
		while(1){ //REVISAR CUANDO TERMINARIA ESTO

			sleep(config_valores.tiempo_sincronizacion);
			pthread_mutex_lock(&sincro_block);
			memcpy(block,copiaBlock,config_valores.block_size * config_valores.blocks);
			pthread_mutex_unlock(&sincro_block);
			int sincro=msync(block, config_valores.block_size * config_valores.blocks, MS_SYNC);
			if(sincro==-1){printf("fallo en la sincronizacion\n");}
			else{printf("se realizo una sincronizacion exitosa\n");}
		}

		close(fd);
		return EXIT_SUCCESS;

	    return;

}

void crear_metadata(){
	crear_matadataOxigeno();
	crear_matadataComida();
	crear_matadataBaura();


}

void crear_matadataOxigeno(){
	log_info(logger, "Buscando Filesystem existente");

		metadata_file_path_oxigeno = string_new();
		string_append(&metadata_file_path_oxigeno, metadata_path);
		string_append(&metadata_file_path_oxigeno, "Oxigeno.ims");

		FILE* metadata_fd = fopen(metadata_file_path_oxigeno, "rb");

		if (metadata_fd != NULL) { //VER SI ESTE IF ESTA BIEN
			fclose(metadata_fd);
				log_info(logger, "Metadata encontrada");
			return;

		}
		log_info(logger, "No encontrado, creando");
		metadata_fd = fopen(metadata_file_path_oxigeno, "w");
		t_config* metadata_config = malloc(sizeof(t_config));
		metadata_config->path = metadata_file_path_oxigeno;
		metadata_config->properties = dictionary_create();
		 char* md5 =calcularMd5(0  , 'O'  );

		dictionary_put(metadata_config->properties, "SIZE", "0"); //VER COMO SE SACA EL SIZE
		dictionary_put(metadata_config->properties, "BLOCK_COUNT", "0");
		dictionary_put(metadata_config->properties, "BLOCKS", "[-1]"); //VER POR QUE NO SE CARGA CORRECTAMENTE
		dictionary_put(metadata_config->properties, "CARACTER_LLENADO", "O");
		dictionary_put(metadata_config->properties, "MD5_ARCHIVO", md5);

		config_save(metadata_config);
		free(md5);
		dictionary_destroy(metadata_config->properties);
		fclose(metadata_fd);




}

void crear_matadataComida(){
	log_info(logger, "Buscando Filesystem existente");

		metadata_file_path_comida = string_new();
		string_append(&metadata_file_path_comida, metadata_path);
		string_append(&metadata_file_path_comida, "Comida.ims");

		FILE* metadata_fd = fopen(metadata_file_path_comida, "rb");

		if (metadata_fd != NULL) { //VER SI ESTE IF ESTA BIEN
			fclose(metadata_fd);
				log_info(logger, "Metadata encontrada");
			return;

		}
		log_info(logger, "No encontrado, creando");
		metadata_fd = fopen(metadata_file_path_comida, "w");
		t_config* metadata_config = malloc(sizeof(t_config));
		metadata_config->path = metadata_file_path_comida;
		metadata_config->properties = dictionary_create();
		 char* md5 =calcularMd5(0  , 'C'  );

		dictionary_put(metadata_config->properties, "SIZE", "0");
		dictionary_put(metadata_config->properties, "BLOCK_COUNT", "0");
		dictionary_put(metadata_config->properties, "BLOCKS", "[-1]");
		dictionary_put(metadata_config->properties, "CARACTER_LLENADO", "C");
		dictionary_put(metadata_config->properties, "MD5_ARCHIVO", md5);

		config_save(metadata_config);
		dictionary_destroy(metadata_config->properties);
		free(metadata_config);
		fclose(metadata_fd);



}

void crear_matadataBaura(){
	log_info(logger, "Buscando Filesystem existente");

		metadata_file_path_basura = string_new();
		string_append(&metadata_file_path_basura, metadata_path);
		string_append(&metadata_file_path_basura, "Basura.ims");

		FILE* metadata_fd = fopen(metadata_file_path_basura, "rb");

		if (metadata_fd != NULL) { //VER SI ESTE IF ESTA BIEN
			fclose(metadata_fd);
				log_info(logger, "Metadata encontrada");
			return;

		}
		log_info(logger, "No encontrado, creando");
		metadata_fd = fopen(metadata_file_path_basura, "w");
		t_config* metadata_config = malloc(sizeof(t_config));
		metadata_config->path = metadata_file_path_basura;
		metadata_config->properties = dictionary_create();
		 char* md5 =calcularMd5(0  , 'B'  );


		dictionary_put(metadata_config->properties, "SIZE", "0");
		dictionary_put(metadata_config->properties, "BLOCK_COUNT", "0");
		dictionary_put(metadata_config->properties, "BLOCKS", "[-1]");
		dictionary_put(metadata_config->properties, "CARACTER_LLENADO", "B");
		dictionary_put(metadata_config->properties, "MD5_ARCHIVO", md5);

		config_save(metadata_config);
		dictionary_destroy(metadata_config->properties);
		free(metadata_config);
		fclose(metadata_fd);


}

void crearBitacora(int id){
	int bloquesLen = -1;
	char* bitacora_tripu = string_new();
	char* idTripu= string_itoa(id);
		string_append(&bitacora_tripu, bitacora_path);
		string_append(&bitacora_tripu, "Tripulante");
		string_append(&bitacora_tripu, idTripu);
		string_append(&bitacora_tripu, ".ims");
	FILE* metadata_fd = fopen(bitacora_tripu, "rb");

	if (metadata_fd != NULL) { //VER SI ESTE IF ESTA BIEN
		fclose(metadata_fd);
		t_config* bitacora_config = config_create(bitacora_tripu);
		char** bloquesUsados = config_get_array_value(bitacora_config, "BLOCKS");

		while (bloquesUsados[++bloquesLen] != NULL){}
		//pasar los bits del numero de bloques a 0
			for(int i=0;i<bloquesLen;i++){
				int bloque=atoi(bloquesUsados[i]);
				bitarray_clean_bit(bitmap,bloque);


			}
		msync(bitmap->bitarray, tamanoBitmap, MS_SYNC);
		log_info(logger, "Metadata encontrada");
		free(bloquesUsados);
		config_destroy(bitacora_config);

	}
	metadata_fd = fopen(bitacora_tripu, "w");
	t_config* metadata_config = malloc(sizeof(t_config));
	metadata_config->path = bitacora_tripu;
	metadata_config->properties = dictionary_create();

	dictionary_put(metadata_config->properties, "SIZE", "0");           //TODO ver si puedo meter esto :D
	dictionary_put(metadata_config->properties, "BLOCKS", "[]");

	config_save(metadata_config);
	dictionary_destroy(metadata_config->properties);
	free(idTripu);
	free(metadata_config);
	free(bitacora_tripu);
	fclose(metadata_fd);
}

