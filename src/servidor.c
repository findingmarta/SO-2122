#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h> 
#include <errno.h>


void retira_pedido (char *** pendentes, int i, int nr_pendentes){
  for (; i< nr_pendentes-1; i++){
    char ** aux = pendentes [i];
    pendentes[i] = pendentes [i+1];
    pendentes[i+i] = aux;
  }
}


void insere_pendentes ( char *pendentes[100][100], int nr_pendentes, char ** args,int n){
    printf("nr_pendentes: %d,  n: %d \n", nr_pendentes, n );
    for(int i =0; i< n; i++){
        pendentes[nr_pendentes][i] = (args[i]);
    }
    for(int i =0; i< n; i++){
        printf("pendentes: %s\n", pendentes[nr_pendentes][i]);
    }

}


int main(int argc, char *argv[]){
    //inicializar o servidor
    char limites[7];
    char *buf=malloc(1024);
    int ft = open (argv[1],O_RDONLY,0640);
    int bytes_read;
    while (( bytes_read = read(ft,buf,1024))>0){
        for(int i=0;i<7 && buf;i++){
            char* str1 = strdup(strsep((&buf)," "));
            int num = atoi(strsep((&buf),"\n"));
            printf("sr1: %s\nnum:%d\n",str1,num);
            if (!strcmp(str1, "nop")) limites[0] = num;
            else if (!strcmp(str1, "bcompress")) limites[1] = num;
            else if (!strcmp(str1, "bdecompress")) limites[2] = num;
            else if (!strcmp(str1, "gcompress")) limites[3] = num;
            else if (!strcmp(str1, "gdecompress"))limites[4] = num;
            else if (!strcmp(str1, "encrypt")) limites[5] = num;
            else if (!strcmp(str1, "decrypt")) limites[6] = num;
        }    
    }
    if (ft==-1) printf("Erro - %d\n", errno);
    close(ft);

    // Create pipe
    char* pendentes[100][100];
    int nr_pendentes = 0;
    if(mkfifo("pipe", 0666)==-1) perror ("fifo_servidor");
    write(1, "criou pipe \n",12);
    int pipe_read = open ("pipe", O_RDONLY);
    int pipe_write = open ("pipe", O_WRONLY);
    char * buffer=malloc(1024);
    while(( bytes_read = read(pipe_read,buffer,1024))>0){
        buffer[bytes_read]='\0';
        buf=strdup(buffer);
        printf("entrou no ciclo :%s\n",buf);
        char* pid = strdup(strsep((&buf)," "));
        char* pedido = strdup(strsep((&buf)," \n"));
        int flag = 1, i,j;

        int pipe_pip = open(pid, O_WRONLY);
        if (!strcmp(pedido, "status")) {
            printf("status\n");
            char *status=malloc(100);
            write(pipe_pip,"pedidos pendentes:\n",19);
            for (i=0;i<nr_pendentes;i++){
                for (j=0; pendentes[i][j];j++) {
                    write(pipe_pip,pendentes[i][j],strlen(pendentes[i][j]));
                    write(pipe_pip," ",1);
                }
                write(pipe_pip,"\n",1);
            }
            sprintf(status,"nop tem disponível %d\n",limites[0]);
            write(pipe_pip,status,strlen(status));
            sprintf(status,"bcompress tem disponível %d\n",limites[1]);
            write(pipe_pip,status,strlen(status));
            sprintf(status,"bdecompress tem disponível %d\n",limites[2]);
            write(pipe_pip,status,strlen(status));
            sprintf(status,"gcompress tem disponível %d\n",limites[3]);
            write(pipe_pip,status,strlen(status));
            sprintf(status,"gdecompress tem disponível %d\n",limites[4]);
            write(pipe_pip,status,strlen(status));
            sprintf(status,"encrypt tem disponível %d\n",limites[5]);
            write(pipe_pip,status,strlen(status));
            sprintf(status,"decrypt tem disponível %d\n",limites[6]);
            write(pipe_pip,status,strlen(status));
            close(pipe_pip);
            //percorrer o array dos pendentes e imprimir
            // percorrer os limites e imprimir
        }

        else if (!strcmp(pedido, "proc-file")) {
            printf(" proc_file \n");
            char* ficheiro_entrada = strdup(strsep((&buf)," "));
            char* ficheiro_saida = strdup(strsep((&buf)," \n"));       
            char* args[100];
            for(i=0; buf && flag; i++){
                args[i] = strdup(strsep((&buf)," \n"));
                if (strlen(args[i])<3) i--;
                else if (!strcmp(args[i], "nop") && limites[0]>0) limites[0]--;
                else if (!strcmp(args[i], "bcompress") && limites[1]>0) limites[1]--;
                else if (!strcmp(args[i], "bdecompress") && limites[2]>0) limites[2]--;
                else if (!strcmp(args[i], "gcompress") && limites[3]>0) limites[3]--;
                else if (!strcmp(args[i], "gdecompress") && limites[4]>0) limites[4]--;
                else if (!strcmp(args[i], "encrypt") && limites[5]>0) limites[5]--;
                else if (!strcmp(args[i], "decrypt") && limites[6]>0) limites[6]--;
                else {
                    printf("pendente\n");
                    for(int j=0; j<i; j++){
                        if (!strcmp(args[j], "nop")) limites[0]++;
                        else if (!strcmp(args[j], "bcompress")) limites[1]++;
                        else if (!strcmp(args[j], "bdecompress")) limites[2]++;
                        else if (!strcmp(args[j], "gcompress")) limites[3]++;
                        else if (!strcmp(args[j], "gdecompress")) limites[4]++;
                        else if (!strcmp(args[j], "encrypt")) limites[5]++;
                        else if (!strcmp(args[j], "decrypt")) limites[6]++;
                    }
                    i++;
                    for(; buf; i++){
                        args[i] = strdup(strsep((&buf)," \n"));
                        if (strlen(args[i])<3) i--;
                    }
                    args[i++] = NULL;
                    args[i++] = ficheiro_entrada;
                    args[i++] = ficheiro_saida;
                    args[i++] = pid;
                    flag = 0;
                    write (pipe_pip, "...pending...\n", 14);
                    close(pipe_pip);
                
                    insere_pendentes ( pendentes, nr_pendentes, args, i);
                    nr_pendentes++;
                }
            }
            printf(" executar \n");

            if(flag && fork()==0){
                sleep(10);
                close (pipe_read);
                write( pipe_pip, "...executing...\n", 16);
                int p[i-1][2];
                int j;
                int f_entrada = open (ficheiro_entrada, O_RDONLY);
                int f_saida = open (ficheiro_saida, O_WRONLY | O_CREAT | O_TRUNC, 0640);
                printf("%d %d\n", f_entrada, f_saida );
                int fd_aux = dup(1);
                dup2(f_entrada,0);
                dup2(f_saida,1);
                close(f_saida);
                close(f_entrada);
                char*cmd=malloc(30);
                strcpy(cmd,argv[2]);
                for (j=0; j<i; j++){
                    if (j==0){
                        pipe(p[j]);
                        if (fork()==0){
                            write(fd_aux,args[j],strlen(args[j]));
                            close (pipe_write);
                            close(pipe_pip);
                            close(p[j][0]);
                            dup2(p[j][1],1);
                            close(p[j][1]);
                            strcat(cmd,args[j]);
                            execlp(cmd, args[j], NULL);
                            write(fd_aux,"deu erro\n",9);
                            _exit(0);
                        }
                        else close(p[j][1]);
                    }
                    else if (j==i-1){
                                 //               printf("entrou no else: %s \n",args[j]);
                        if (fork()==0){
                            write(fd_aux,args[j],strlen(args[j]));
                            close (pipe_write);
                            close(pipe_pip);
                            dup2(p[j-1][0],0);
                            close(p[j-1][0]);
                            strcat(cmd,args[j]);
                            execlp(cmd, args[j], NULL);
                            write(fd_aux,"deu erro\n",9);
                            _exit(0);
                        }
                        else close(p[j-1][0]);
                    }
                    else {
                        pipe(p[j]);
                        if (fork()==0){
                            write(fd_aux,args[j],strlen(args[j]));
                            close (pipe_write);
                            close(pipe_pip);
                            close(p[j][0]);
                            dup2(p[j][1],1);
                            close(p[j][1]);
                            dup2(p[j-1][0],0);
                            close(p[j-1][0]);
                            strcat(cmd,args[j]);
                            execlp(cmd, args[j], NULL);
                            write(fd_aux,"deu erro\n",9);
                            _exit(0);
                        }
                        else {
                            close(p[j][1]);
                            close(p[j-1][0]);
                        }
                    }
                }
                for (j=0; j<i; j++) wait(NULL);
                write (pipe_pip, "...finished...\n", 15); 


                char * pendente = malloc(100);
                strcpy(pendente,pid);
                strcat(pendente," Done ");
                for (int k=0; k<i;k++){
                    strcat(pendente,args[k]);
                    strcat(pendente," ");
                }
                strcat(pendente,"\n");
                write(pipe_write,pendente,strlen(pendente));
                close (pipe_write);
                close(pipe_pip);
                _exit(0);
                
            }
            close(pipe_pip);
        }

        else if (!strcmp(pedido, "Done")){
        printf ("entrouuuuu: %s\n", buf);
        close(pipe_pip);
            while (buf){
               char * op =  strdup(strsep((&buf)," \n"));
                if (!strcmp(op, "nop")) limites[0]++;
                else if (!strcmp(op, "bcompress")) limites[1]++;
                else if (!strcmp(op, "bdecompress")) limites[2]++;
                else if (!strcmp(op, "gcompress")) limites[3]++;
                else if (!strcmp(op, "gdecompress")) limites[4]++;
                else if (!strcmp(op, "encrypt")) limites[5]++;
                else if (!strcmp(op, "decrypt")) limites[6]++;
            }
            printf("limites nop: %d\n", limites[0]);

                int aux[7];
                for (i=0; i<7; i++) aux[i] = limites[i];
                    printf("nr_pendentes: %d \n", nr_pendentes );
                for(i=0; i<nr_pendentes ; i++){
                    flag = 1;
    
                    for (j=0; pendentes[i][j] != NULL && flag; j++) {

                         if (!strcmp(pendentes[i][j], "nop") && aux[0]>0) aux[0]--;
                         else if (!strcmp(pendentes[i][j], "bcompress") && aux[1]>0) aux[1]--;
                         else if (!strcmp(pendentes[i][j], "bdecompress") && aux[2]>0) aux[2]--;
                         else if (!strcmp(pendentes[i][j], "gcompress") && aux[3]>0) aux[3]--;
                         else if (!strcmp(pendentes[i][j], "gdecompress") && aux[4]>0) aux[4]--;
                         else if (!strcmp(pendentes[i][j], "encrypt") && aux[5]>0) aux[5]--;
                         else if (!strcmp(pendentes[i][j], "decrypt") && aux[6]>0) aux[6]--;
                         else {
                            printf("i:%d , pendentes_ %s\n", i, pendentes[i][j] );
                             for(j=0; j<i; j++){
                                 if (!strcmp(pendentes[i][j], "nop")) aux[0]++;
                                 else if (!strcmp(pendentes[i][j], "bcompress")) aux[1]++;
                                 else if (!strcmp(pendentes[i][j], "bdecompress")) aux[2]++;
                                 else if (!strcmp(pendentes[i][j], "gcompress")) aux[3]++;
                                 else if (!strcmp(pendentes[i][j], "gdecompress")) aux[4]++;
                                 else if (!strcmp(pendentes[i][j], "encrypt")) aux[5]++;
                                 else if (!strcmp(pendentes[i][j], "decrypt")) aux[6]++;
                             }

                             flag = 0;


                        }
                    }

                    if (flag){
                        printf("entrou flag\n");
                        //criar o pedido
                        char *pedido = malloc(2000);
                        strcpy (pedido, pendentes[i][j+3]);
                        strcat (pedido, " proc-file ");
                        strcat (pedido, pendentes[i][j+1]);
                        strcat (pedido, " ");
                        strcat (pedido, pendentes[i][j+2]);
                        strcat (pedido, " ");

                        for (int c = 0; c < j; c++){  
                            strcat(pedido,pendentes[i][c]);
                            strcat(pedido, " ");
                        }
                        strcat(pedido, "\n");

                        write(pipe_write, pedido, strlen(pedido));
                        retira_pedido ( pendentes, i--, nr_pendentes--);

                    }

                }
                
            }
            else printf("nao entrou em nada  %s\n", pedido);
    

    }
    printf("Saiu: %d - %d",bytes_read,errno);
}
