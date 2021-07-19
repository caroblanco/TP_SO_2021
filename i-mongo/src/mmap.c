/*#include <stdio.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main() {
    char *addr, file_name;
    int fd, cantAAgregar, cuantosBytes, cuantasO;
    cuantosBytes = 132;
    cuantasO = 0;
    fd = open(file_name, O_RDWR);
    addr = mmap(NULL, cuantosBytes, PROT_WRITE, MAP_SHARED, fd, 0);
    return 0;

    for(size_t i = 0; i < (cuantosBytes/sizeof(char)); i++) {
        if(addr[i] == 'O'){
            cuantasO ++;
        }
    }
    for(size_t i = cuantasO; i < cantAAgregar + cuantasO; i++) {
        addr[i] = 'O';
    }
    printf("El texto final es:\n %s", addr);
    return 0;
}



#include <stdio.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include<commons/log.h>
#include<commons/string.h>

int main() {
    char* addr;
    char* file_name = "text.txt";
    int fd, cantAAgregar, cuantosBytes, cuantasO;
    cantAAgregar = 10;
    cuantosBytes = 132;
    cuantasO = 0;
    fd = open(file_name, O_RDWR);
    addr = mmap(NULL, cuantosBytes, PROT_WRITE, MAP_SHARED, fd, 0);


    for(size_t i = 0; i < (cuantosBytes/sizeof(char)); i++) {
        if(addr[i] == 'O'){
            cuantasO ++;
        }
    }
    for(size_t i = cuantasO; i < cantAAgregar + cuantasO; i++) {
        addr[i] = 'O';
    }
    printf("El texto final es:\n %s", addr);
    return 0;
}*/
