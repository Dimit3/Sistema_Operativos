#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/stat.h>
#include "writetxt.h"
#include "readln.h"
#include "tratar.h"
#include <stdio.h>



int main (int argc, char * argv[]){
    int fifo_fd;
    char buf[1024];
    int b = 0;
    int w = 0; 
    char ret[1024]; 
    if (argc == 1){
        
        writetxt(0,"argus$ ");
        while ( (b = readln(0,buf,1024)) > 0){
            w=tratar(buf,b,ret);
            
            if (w == -1){
                writetxt(1,"Comando invalido, escreva ajuda para imprimir a ajuda!\n");
                return 0;
            }
            if (strncmp(ret,"-h ",3)==0){
                writetxt(1,"tempo-inactividade segs || -i segs -> o tempo maximo (segundos) de inactividade de comunicacao num pipe anonimo\n\0");
                writetxt(1,"tempo-execucao segs || -m segs -> o tempo maximo (segundos) de execucao de uma tarefa\n\0");
                writetxt(1,"executar \"p1 | p2 ... | pn\" || -e \"p1 | p2 ... | pn\" -> executar uma tarefa\n\0");
                writetxt(1,"listar || -l-> listar tarefas em execucao\n\0");
                writetxt(1,"terminar n || -t n -> terminar uma tarefa em execucao\n\0");
                writetxt(1,"historico || -r -> listar registo historico de tarefas terminadas\n\0");
                writetxt(1,"ajuda || -h -> apresentar ajuda a sua utilizacao\n\0");
                writetxt(1,"output -n || -o n -> apresentar o output da tarefa n\n\0");
                return 0;
            }
            else if (strncmp(ret,"-l ",3)==0 || strncmp(ret,"-r ",3)==0 ){
                if((fifo_fd=open("fifo",O_WRONLY))<0){
                    perror("fifo open");
                    exit(1);
                }
                mkfifo("fifo1",0666);
                int fifo1_fd = 0;
                ret[3]='\n';
                write(fifo_fd,ret,4);
                if((fifo1_fd=open("fifo1",O_RDONLY))<0){
                    perror("fifo open");
                    exit(1);
                }
                
                char listar[100];
                int read_bytes = 0;
                while((read_bytes = read(fifo1_fd,listar,100)) > 0){
                    write(1,listar,read_bytes);
                }
                close(fifo1_fd);

            }

            else if (strncmp(ret,"-o ",3)==0){
                if((fifo_fd=open("fifo",O_WRONLY))<0){
                    perror("fifo open");
                    exit(1);
                }
                if(write(fifo_fd,ret, w+1)<0){
                        perror("write fifo");
                }
                int fifo1_fd;
                mkfifo("fifo1",0666);
                if((fifo1_fd=open("fifo1",O_RDONLY))<0){
                    perror("fifo open");
                    exit(1);
                }
                char listar[100];
                int read_bytes = 0;
                while((read_bytes = read(fifo1_fd,listar,100)) > 0){
                    write(1,listar,read_bytes);
                }
                close(fifo1_fd);
            }
            
            else{
                if((fifo_fd=open("fifo",O_WRONLY))<0){
                    perror("fifo open");
                    exit(1);
                }
                if(write(fifo_fd,ret, w+1)<0){
                        perror("write fifo");
                    }
                close(fifo_fd);
            }

            writetxt(0,"\nargus$ ");

        } 
    }else{ 
        if (strcmp(argv[1],"-i")==0 ||strcmp(argv[1],"-m")==0 || strcmp(argv[1],"-e")==0 ||strcmp(argv[1],"-t")==0 ||strcmp(argv[1],"-l")==0 ||strcmp(argv[1],"-r")==0 ||strcmp(argv[1],"-h")==0 ||strcmp(argv[1],"-o")==0 ){


            if(strcmp(argv[1],"-h")==0){
                writetxt(1,"tempo-inactividade segs || -i segs -> o tempo maximo (segundos) de inactividade de comunicacao num pipe anonimo\n\0");
                writetxt(1,"tempo-execucao segs || -m segs -> o tempo maximo (segundos) de execucao de uma tarefa\n\0");
                writetxt(1,"executar \"p1 | p2 ... | pn\" || -e \"p1 | p2 ... | pn\" -> executar uma tarefa\n\0");
                writetxt(1,"listar || -l-> listar tarefas em execucao\n\0");
                writetxt(1,"terminar n || -t n -> terminar uma tarefa em execucao\n\0");
                writetxt(1,"historico || -r -> listar registo historico de tarefas terminadas\n\0");
                writetxt(1,"ajuda || -h -> apresentar ajuda a sua utilizacao\n\0");
                writetxt(1,"output -n || -o n -> apresentar o output da tarefa n\n\0");
                return 0;
            }
            else if (strcmp(argv[1],"-l")==0 || strcmp(argv[1],"-r")==0){
                if((fifo_fd=open("fifo",O_WRONLY))<0){
                    perror("fifo open");
                    exit(1);
                }
                mkfifo("fifo1",0666);
                int fifo1_fd = 0;
                strcat(argv[1],"\n");
                write(fifo_fd,argv[1],4);
                if((fifo1_fd=open("fifo1",O_RDONLY))<0){
                    perror("fifo open");
                    exit(1);
                }
                
                char listar[100];
                int read_bytes = 0;
                while((read_bytes = read(fifo1_fd,listar,100)) > 0){
                    write(1,listar,read_bytes);
                }
                close(fifo1_fd);
            }

            else if (strcmp(argv[1],"-o")==0){
                if((fifo_fd=open("fifo",O_WRONLY))<0){
                    perror("fifo open");
                    exit(1);
                }
                char comando [1024];
                for (int h=0;h<1024;h++){comando[h]='\0';}
                for (int j=1;j<argc;j++){
                    strcat(comando,argv[j]);
                    if (j==argc-1){
                        strcat(comando,"\n");
                    }else{
                        strcat(comando," ");
                    }
                }
                int s = strlen(comando);
                write(fifo_fd,comando,s);
            
                int fifo1_fd;
                mkfifo("fifo1",0666);
                if((fifo1_fd=open("fifo1",O_RDONLY))<0){
                    perror("fifo open");
                    exit(1);
                }
                char listar[100];
                int read_bytes = 0;
                while((read_bytes = read(fifo1_fd,listar,100)) > 0){
                    write(1,listar,read_bytes);
                }
                close(fifo1_fd);
                return 0;

            }else{
                if((fifo_fd=open("fifo",O_WRONLY))<0){
                    perror("fifo open");
                    exit(1);
                }

                char comando [1024];
                for (int h=0;h<1024;h++){comando[h]='\0';}
                for (int j=1;j<argc;j++){
                    strcat(comando,argv[j]);
                    if (j==argc-1){
                        strcat(comando,"\n");
                    }else{
                        strcat(comando," ");
                    }
                }
                int s = strlen(comando);
                write(fifo_fd,comando,s);
            
            }
        }
        else{
            writetxt(1,"comando invalido escolha a opçao '-h' para ajuda\n");
            return 0;
        }
    }
    close(fifo_fd);
    return 0;
    
}