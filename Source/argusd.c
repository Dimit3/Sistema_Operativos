#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include "writetxt.h"
#include "readln.h"
#include <string.h>
#include <signal.h>
#include <sys/wait.h>

struct Tarefa {
    int numtarefa;
    int inicio;
    int output;
    int *pids;
    int total;
    char * tarefa;
    int comandos;
    int tamoutput;
    int posidx;
    char estado;  //e- execuçao   t - terminada pelo utilizador 
                  //   i - terminada por inatividade 
                    // m -   terminada por tempo maximo 
                    // a - acabou normalmente
                    // k - invalido
};

static struct Tarefa tarefas[100];
static int tempoExecucao=100000;
static int poslog = 0;
static int posidx = 0;
static int tempoInatividade=100000;

void timeout_handler(int sig){
    for (int x=0;x<100;x++){
        if (tarefas[x].estado=='e'){
            tarefas[x].total++;
            if (tarefas[x].total>=tempoExecucao){
                tarefas[x].estado='m';
                for (int i=0;i< tarefas[x].comandos;i++){
                    if(tarefas[x].pids[i]>0){
                        kill(tarefas[x].pids[i],SIGKILL);
                    }
                }
                free(tarefas[x].pids);
            }
        }
    }
    alarm(1);
}

void inatividade_handler(int signum){
    _exit(10);
}
void sigchld_handler(int signum){
    int status;
    int pid;
    pid= wait(&status);
    int i;
    for (i=0;i<100;i++){
        if (tarefas[i].estado=='e'){
            for(int x=0;x<tarefas[i].comandos;x++){
                if(tarefas[i].pids[x]==pid){
                    if (WEXITSTATUS(status)==10){
                        for(int r=0;r<tarefas[i].comandos;r++){
                            if(tarefas[x].pids[i]>0){
                                kill(tarefas[i].pids[r],SIGKILL);
                            }
                        }
                        free(tarefas[i].pids);
                        tarefas[i].estado='i';
                    }else{
                        free(tarefas[i].pids);
                        tarefas[i].estado='a';
                        int file_fd;
                        file_fd = open ("log" , O_RDONLY | O_APPEND);
                        int dif = lseek (file_fd , 0 , SEEK_END);
                        tarefas[i].tamoutput = dif - poslog;
                        file_fd = open ("log.idx" , O_WRONLY | O_CREAT | O_APPEND , 0666);
                        tarefas[i].posidx = posidx + 1;
                        char buf[10];
                        for (int ll = 0; ll < 10 ; ll++) buf[ll] = '\0';
                        sprintf (buf , "%d" , poslog + 1);
                        strcat (buf," ");
                        write (file_fd, buf , strlen(buf));
                        poslog = dif;
                        posidx += strlen(buf);
                    }
                    break;
                }
            }
        }
    }
    
}

int atual;

int main (int argc, char * argv[]){
    if(signal(SIGCHLD,sigchld_handler)==SIG_ERR){
        perror("SIGCHLD failed");
        exit(1);
    }
    if(signal(SIGALRM,timeout_handler)==SIG_ERR){
        perror("SIGALRM failed");
        exit(1);
    }
    for (int x = 0; x < 100; x ++){
        tarefas[x].estado = 'e';
    }
    int fifo1_fd = 0;
    int contador = 0;
    mkfifo("fifo",0666);
    int fifo_fd;
    char aux[100];
    if((fifo_fd=open("fifo",O_RDONLY))<0){
        perror("fifo open");
        exit(1);
    }
    char buf[100];
    for (int h=0;h<100;h++){
        buf[h]='\0';}
    int bytes_read;

    while((bytes_read=readln(fifo_fd,buf,100))>0){
        int pipe_fd[100][2];
        switch ((buf[1])){
            case 'i':
                tempoInatividade = atoi(buf+3);
                break;

            case 'm':
                tempoExecucao = atoi(buf+3);
                break;
            
            case 'e':;
                atual = contador;
                tarefas[atual%100].numtarefa = contador+1;
                tarefas[atual%100].tarefa = malloc(bytes_read);
                strncpy(tarefas[atual%100].tarefa,buf+3,bytes_read-4);
                tarefas[atual%100].estado = 'e';
                contador ++;
                char * novaS;
                char * sep [30];
                int i = 0;
                novaS = strtok ( buf , " \'\"" );
                

                while (novaS != NULL)
                {
                    sep[i++] = novaS;
                    novaS = strtok (NULL, " \n\'\"");
                }
                sep[i] = "FIM";
                i = 1;
                int commands , j;
                commands = j = 0;
                char * execs [20][10];

                while ( strcmp(sep[i],"FIM")!=0 ){

                    if ( strcmp(sep[i],"|")==0){
                    
                    execs[commands][j] = NULL;
                    commands++;
                    j=0;}
                    
                    else{
                    execs[commands][j] = sep[i];
                    j++;}

                    i++;
                    
                    }
                execs[commands][j] = NULL;
                tarefas[atual%100].comandos = commands+1;
                tarefas[atual%100].pids = malloc(sizeof(int)*(commands+1));
                if (commands >=1){
                    i = 0;
                    
                    if(pipe(pipe_fd[0])<0){
                        perror("pipe[0]");
                        exit(1);
                    }
                    int pid;
                    switch (pid = fork()){

                        case 0:
                            close(pipe_fd[0][0]);
                            dup2(pipe_fd[0][1],1);
                            close(pipe_fd[0][1]);
                            execvp(execs[0][0] , execs[0]);
                            _exit(1);
                            break;
                        
                        case -1:
                            perror("fork");
                            exit(1);
                            break;
                        
                        default:
                            tarefas[atual%100].pids[0] = pid;
                            close (pipe_fd[0][1]);
                            break;

                    }

                    
                    if(pipe(pipe_fd[1])<0){
                        perror("pipe[1]");
                        exit(1);
                    }
                    int w;
                    int sensor = 1;
                    switch(pid = fork()){

                        case 0:
                        
                            w = 0;
                            close (pipe_fd[1][0]);
                            if(signal(SIGALRM,inatividade_handler)==SIG_ERR){
                                perror("SIGALRM failed");
                                exit(1);
                            }
                            
                            alarm(tempoInatividade);
                            while ((w = (read(pipe_fd[0][0],buf,100))) > 0){
                                alarm(tempoInatividade);
                                write (pipe_fd[1][1],buf,w);
                            }
                            break;

                        case -1:
                            perror("fork");
                            exit(1);
                            break;
                        default:
                            tarefas[atual%100].pids[i+sensor] = pid;
                            close(pipe_fd[0][0]);
                            close(pipe_fd[1][1]);
                            break;

                    }

                    for(i=1;i<commands;i++){


                        if(pipe(pipe_fd[i+sensor])<0){
                            perror("pipe[i]");
                            exit(1);
                        }

                        switch (pid = fork()){

                        case 0:
                            close(pipe_fd[i+sensor][0]);
                            dup2(pipe_fd[i+sensor-1][0],0);
                            close(pipe_fd[i+sensor-1][0]);
                            dup2(pipe_fd[i+sensor][1],1);
                            close(pipe_fd[i+sensor][1]);
                            execvp(execs[i][0] , execs[i]);
                            _exit(1);
                            break;
                        
                        case -1:
                            perror("fork");
                            exit(1);
                            break;
                        
                        default:
                            close(pipe_fd[i+sensor-1][0]);
                            close(pipe_fd[i+sensor][1]);
                            tarefas[atual%100].pids[i+sensor] = pid;
                            break;
                        sensor++;
                       
                        switch(pid = fork()){

                        case 0:
                            
                            w = 0;
                            close (pipe_fd[i+sensor+1][0]);
                            if(signal(SIGALRM,inatividade_handler)==SIG_ERR){
                                perror("SIGALRM failed");
                                exit(1);
                            }
                            
                            alarm(tempoInatividade);
                            while ((w = (read(pipe_fd[i+sensor][0],buf,100))) > 0){
                                alarm(tempoInatividade);
                                write (pipe_fd[i+sensor+1][1],buf,w);
                            }
                            break;
                        case -1:
                            perror("fork");
                            exit(1);
                            break;
                        default:
                            tarefas[atual%100].pids[i+sensor] = pid;
                            close(pipe_fd[i+sensor-1][0]);
                            close(pipe_fd[i+sensor][1]);
                            break;
                    }

                    } }

                    switch (pid = fork()){

                        case 0:;
                            int file_fd;
                            if((file_fd=open("log",O_WRONLY|O_APPEND|O_CREAT,0666))<0){
                                perror("file open");
                                exit(1);
                                
                            }
                            dup2(file_fd,1);
                            close(file_fd);
                            dup2(pipe_fd[i+sensor-1][0],0);
                            close(pipe_fd[i+sensor-1][0]);
                            execvp(execs[i][0] , execs[i]);
                            _exit(1);
                            break;
                        
                        case -1:
                            perror("fork");
                            exit(1);
                            break;
                        
                        default:
                            close (pipe_fd[i+sensor-1][0]);
                            tarefas[atual%100].pids[i+sensor] = pid;
                            break;

                    }
                    tarefas[atual].comandos+=sensor;
                }else{
                    int pid;
                    switch (pid = fork()){

                        case 0:;
                            int file_fd;
                            if((file_fd=open("log",O_WRONLY|O_APPEND|O_CREAT,0666))<0){
                                perror("file open");
                                exit(1);
                                
                            }
                            dup2(file_fd,1);
                            close(file_fd);
                            execvp(execs[0][0] , execs[0]);
                            _exit(1);
                            break;
                        
                        case -1:
                            perror("fork");
                            exit(1);
                            break;
                        
                        default:
                            tarefas[atual%100].pids[0] = pid;
                            break;
                    }
                }
                alarm(1);
                break;
            
            case 'l':;
                
                if((fifo1_fd=open("fifo1",O_WRONLY))<0){
                    perror("fifo open");
                    exit(1);
                }
                

                for (int y = 0; y < contador; y++){
                    for (int x = 0; x < 100; x++){
                        aux[x] = '\0';
                    }
                    aux[0] = '#';
                    if (tarefas[y].estado == 'e'){
                        sprintf(aux+1,"%d",y+1);
                    
                        for (int z = 2; z < 6; z++){
                            if (aux[z] == '\0'){
                                aux[z] = ':';
                                aux[z+1] = ' ';
                                break;
                            }
                        }
                        strcat(aux,tarefas[y].tarefa);
                        strcat(aux,"\n");
                        write(fifo1_fd,aux,strlen(aux));
                    }
                    
                }
                close(fifo1_fd);
                break;
            
            case 't':;
                // -t 10
                
                int numeroDaTarefa = atoi(buf+3)-1;
                if (numeroDaTarefa > 100){
                    writetxt(1,"erro numero de tarefa invalido");
                }
                else if (tarefas[numeroDaTarefa].estado == 'e'){
                    for (int x = 0; x < tarefas[numeroDaTarefa].comandos; x++){
                        kill(tarefas[numeroDaTarefa].pids[x],SIGKILL);
                    }
                }
                free(tarefas[numeroDaTarefa].pids);
                tarefas[numeroDaTarefa].estado = 't';
                break;
            
            case 'r':;
                if((fifo1_fd=open("fifo1",O_WRONLY))<0){
                    perror("fifo open");
                    exit(1);
                }

                for (int y = 0; y < 100; y++){
                    for (int x = 0; x < 100; x++){
                        aux[x] = '\0';
                    }
                    aux[0] = '#';
                    if (tarefas[y].estado != 'k' && tarefas[y].estado != 'e'){
                        sprintf(aux+1,"%d",y+1);

                        for (int z = 2; z < 6; z++){
                            if (aux[z] == '\0'){
                                aux[z] = ',';
                                aux[z+1] = ' ';
                                break;
                            }
                        }
                        switch (tarefas[y].estado)
                        {
                        case 'a':
                            strcat(aux,"concluida: ");
                            break;
                        case 't':
                            strcat(aux,"terminada pelo utilizador: ");
                            break;
                        case 'm':
                            strcat(aux,"max execução: ");
                            break;
                        case 'i':
                            strcat(aux,"max inactividade: ");
                            break;
                        default:
                            break;
                        }

                        strcat(aux,tarefas[y].tarefa);
                        strcat(aux,"\n");
                        write(fifo1_fd,aux,strlen(aux));
                    }

                }
                close(fifo1_fd);
                break;
           
            case 'o':;

                int ntarefa = atoi(buf+3)-1;
                char * buff;
                buff = malloc ( sizeof(char) * tarefas[ntarefa].tamoutput);
                char aux [10];
                int x;
                for (x  = 0; x < 10 ; x++) aux[x] = '\0';
                int lidos = 0;
                int file_fd = open ("log.idx",O_RDONLY);
                lseek (file_fd , tarefas[ntarefa].posidx , SEEK_SET);
                read (file_fd , aux , 10);
                for ( x = 0; x < 10 ; x++){
                    if (aux[x] == ' '){
                        aux[x] = '\0';
                        break;
                    }
                }
                int posLog = atoi(aux);
                file_fd = open ("log",O_RDONLY);
                lseek (file_fd , posLog , SEEK_SET);
                lidos = read (file_fd , buff , tarefas[ntarefa].tamoutput);

                if((fifo1_fd=open("fifo1",O_WRONLY))<0){
                    perror("fifo open");
                    exit(1);
                }

                write(fifo1_fd,buff,lidos);
                close(fifo1_fd);
                break;
            default:
                break;
        }
        close(fifo_fd);
        if((fifo_fd=open("fifo",O_RDONLY))<0){
            perror("fifo open");
            exit(1);}
        for (int x = 0; x < 100; x++){
                        buf[x] = '\0';
                    }
        
    }
    close(fifo_fd);
    return 0;
}




