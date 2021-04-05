#include <stdlib.h>
#include <unistd.h>
#include <string.h>




int codifica (char *codigo, char *comando){
    comando[0] = '-';
    comando[2] = ' ';

    if (strncmp (codigo,"tempo-inactividade",17) == 0){
        comando[1] = 'i';
        return 1;
    }
    if (strncmp (codigo,"tempo-execucao",14) == 0){
        comando[1] = 'm';
        return 1;
    }
    if (strncmp (codigo,"executar",8) == 0){
        comando[1] = 'e';
        return 1;
    }
    if (strncmp (codigo,"listar",6) == 0){
        comando[1] = 'l';
        return 1;
    }
    if (strncmp (codigo,"terminar",8) == 0){
        comando[1] = 't';
        return 1;
    }
    if (strncmp (codigo,"historico",9) == 0){
        comando[1] = 'r';
        return 1;
    }
    if (strncmp (codigo,"output",6) == 0){
        comando[1] = 'o';
        return 1;
    }
    if (strncmp (codigo,"ajuda",5) == 0){
        comando[1] = 'h';
        return 1;
    }
    return 0;
}



int tratar(char buf[], int tam, char ret[]){
    int f=0;
    int i;
    for(i=0;i<tam && f == 0;i++){
        if (buf[i] == ' ')
            f=1;
    }

    char exec[tam-i-1];
    for (int g = i, f = 0; g < tam; g++,f++){
        exec[f] = buf[g];
    }
    
    int j;
    char metodo[i-1];
    for (j= 0; j < i-1; j++){
        metodo[j] = buf[j];
    }

    int sucesso = codifica (metodo,ret);
    if (sucesso == 0){
        return -1;
    }
    else {
        for (int k = 3, a = 0; k < (tam-i+3); k++ , a++ ){
            ret[k] = exec[a];
        }
    }
    return tam-i+2;
}
