#include <string.h>
#include <fcntl.h>
#include <unistd.h>
static int pos = 0;
static int read_bytes = 0;
static char buffer [1024];


int readchar (int fd, char *c){
	if (pos == read_bytes){
		read_bytes = read(fd,buffer,1024);
		pos = 0;
		if (read_bytes == 0) return 0;
	}
	
	*c = buffer[pos];
	pos++;
	return 1;
}

int readln (int fd, char *line, int size){

	int i = 0;
	char c;
	while((readchar(fd,&c))>0){
		line[i] = c;
        i++;
		if (c=='\n')
			break;
	}

	return i;
}	
