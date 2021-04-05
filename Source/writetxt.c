#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
int writetxt(int fd,char *texto){
    return write(fd,texto,strlen(texto));
}