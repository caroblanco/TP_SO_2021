#ifndef SABOTAJES_H
#define SABOTAJES_H


#include "imongo.h"
#include <commons/collections/dictionary.h>
#include <string.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

#include<math.h>
#include<commons/log.h>
#include<commons/string.h>

void reiniciarBitmapEnCero(int  );
void abrirBitacoras(int  );
int contarBlock();


#endif /* SABOTAJES_H */
