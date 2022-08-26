#include <sys/types.h>
#include <sys/wait.h>
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


/* 
    Recebe a matriz com os pedidos pendentes.
    Recebe o nº de pedidos pendentes.
    Recebe o array args com os argumentos.
    Recebe o n que é o número de elementos preenchidos no array args (!= tamanho do array) 
        -> argumentos, NULL, ficheiro de entrada, ficheiro de saída e pid

    Vai preencher a matriz com a informação necessária
*/
void insere_pendentes ( char *pendentes[100][100], int nr_pendentes, char ** args,int n){
    for(int i =0; i< n; i++){
        pendentes[nr_pendentes][i] = (args[i]); // começa a preencher o pedido com as transformações do mesmo
    }
}

int bytes (char *f){
  int fd = open(f,O_RDONLY);
  int r = (int) lseek(fd,0,SEEK_END);
  close(fd);
  return r;
}


int main(int argc, char *argv[]){
    //inicializar o servidor
    char limites[7];
    char *buf=malloc(1024);                    // aloca memória para um buffer responsável por armazenar o resultado da leitura de cada linha do ficheiro com os limites
    int ft = open (argv[1],O_RDONLY,0640);     // file descriptor do ficheiro txt (com os limites)
    int bytes_read;
    while (( bytes_read = read(ft,buf,1024))>0){
        for(int i=0;i<7 && buf;i++){
            char* transf = strdup(strsep((&buf)," "));                //começa a fazer o parsing de cada linha do ficheiro (assume-se que este é válido)
            int lim = atoi(strsep((&buf),"\n"));
            if (!strcmp(transf, "nop")) limites[0] = lim;             //começa a inicializar o array com as transformações e os respetivos limites
            else if (!strcmp(transf, "bcompress")) limites[1] = lim;
            else if (!strcmp(transf, "bdecompress")) limites[2] = lim;
            else if (!strcmp(transf, "gcompress")) limites[3] = lim;
            else if (!strcmp(transf, "gdecompress"))limites[4] = lim;
            else if (!strcmp(transf, "encrypt")) limites[5] = lim;
            else if (!strcmp(transf, "decrypt")) limites[6] = lim;
        }    
    }
    close(ft);

    
    char* pendentes[100][100];                                // cria-se uma matriz para armazenar os pedidos pendentes
    int nr_pendentes = 0;              

    if(mkfifo("pipe", 0666)==-1) perror ("fifo_servidor");    // criar o pipe com nome para receber os pedidos dos clientes 
    int pipe_read = open ("pipe", O_RDONLY);                  // abre o pipe para leitura e aguarda os pedidos (bloqueado)
    int pipe_write = open ("pipe", O_WRONLY);                 // abre o pipe para escrita para que o servidor não termine a sua execução após um pedido
    
    char * buffer=malloc(1024);                               // aloca memória para um buffer responsável pela leitura de cada pedido
    while(( bytes_read = read(pipe_read,buffer,1024))>0){
        buffer[bytes_read]='\0';                              // o \0 é só para indicar o final do pedido  
        buf=strdup(buffer);
        char* pid = strdup(strsep((&buf)," "));               // vai buscar o nome do pipe que corresponde ao pid do cliente 
        char* pedido = strdup(strsep((&buf)," \n"));          // vai buscar o pedido feito pelo cliente
        int flag = 1, i,j;                                    // a flag servirá para indicar se o pedido pode ser executado (=1) ou se fica pendente (=0)


        int pipe_pip;                                         
        
        if( strcmp(pedido, "Done")) pipe_pip = open(pid, O_WRONLY);  // é aberto pipe com nome criado no cliente no modo escrita pois queremos enviar info a esse cliente 
        
        if (!strcmp(pedido, "status")) {                      // começa a processar o pedido status
            char *status=malloc(100);
            write(pipe_pip,"pedidos pendentes:\n",19);        // informa o cliente sobre os pedidos pendentes  
            for (i=0;i<nr_pendentes;i++){
                for (j=0; pendentes[i][j];j++) {
                    write(pipe_pip,pendentes[i][j],strlen(pendentes[i][j]));        // informa o cliente qual é a transformação do pedido que está pendente
                    write(pipe_pip," ",1);
                }
                write(pipe_pip,"\n",1);
            }
            sprintf(status,"nop tem disponível %d\n",limites[0]);           // começa a juntar no status a informação toda sobre o estado de utilização das transformações 
            write(pipe_pip,status,strlen(status));                          // e envia-a para o respetivo cliente
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
            close(pipe_pip);                                                // fechamos o pipe do cliente porque o pedido foi concluido
        }

        else if (!strcmp(pedido, "proc-file")) {                            // começa a processar o pedido proc-file
            char* ficheiro_entrada = strdup(strsep((&buf)," "));            // vai buscar o ficheiro a ser transformado
            char* ficheiro_saida = strdup(strsep((&buf)," \n"));            // vai buscar o ficheiro de destinado a guardar o resultado 
            char* args[100];
            
            //comecamos por analisar cada transformação do pedido e ajustar a quantidade utilizada de cada uma 
            for(i=0; buf && flag; i++){                                     // a flag a 1 pode executar o pedido
                args[i] = strdup(strsep((&buf)," \n"));                     // o buf nesta altura só tem as transformações e o programa trata de ir buscá-las uma a uma
                if (strlen(args[i])<3) i--;                                 // ou seja se for uma transformação inválida ele guarda o próximo (se houver) pedido na mesma posição
                                                                            // caso contrário diminuimos o limite correspondente ao tipo de transformação indicando que esta está a ser executada 
                else if (!strcmp(args[i], "nop") && limites[0]>0) limites[0]--;
                else if (!strcmp(args[i], "bcompress") && limites[1]>0) limites[1]--;
                else if (!strcmp(args[i], "bdecompress") && limites[2]>0) limites[2]--;
                else if (!strcmp(args[i], "gcompress") && limites[3]>0) limites[3]--;
                else if (!strcmp(args[i], "gdecompress") && limites[4]>0) limites[4]--;
                else if (!strcmp(args[i], "encrypt") && limites[5]>0) limites[5]--;
                else if (!strcmp(args[i], "decrypt") && limites[6]>0) limites[6]--;
                
                else {                                                      // se não der para ajustar, isto é, o limite da transformação for atingido o pedido fica pendente               
                    for(int j=0; j<i; j++){                                 // como o pedido fica todo pendente quando determinada transformação atinge um limite temos que aumentar este valor nas transformações que foram aceites antes
                        if (!strcmp(args[j], "nop")) limites[0]++;
                        else if (!strcmp(args[j], "bcompress")) limites[1]++;
                        else if (!strcmp(args[j], "bdecompress")) limites[2]++;
                        else if (!strcmp(args[j], "gcompress")) limites[3]++;
                        else if (!strcmp(args[j], "gdecompress")) limites[4]++;
                        else if (!strcmp(args[j], "encrypt")) limites[5]++;
                        else if (!strcmp(args[j], "decrypt")) limites[6]++;
                    }

                    i++;                                                    // vamos buscar as próximas transformações para tornar o pedido todo pendente  
                    for(; buf; i++){
                        args[i] = strdup(strsep((&buf)," \n"));
                        if (strlen(args[i])<3) i--;
                    }
                    args[i++] = NULL;                                       // indicações extra
                    args[i++] = ficheiro_entrada;
                    args[i++] = ficheiro_saida;
                    args[i++] = pid;
                    flag = 0;                                               // flag a 0 para indicar que o pedido não será executado

                    write (pipe_pip, "...pending...\n", 14);
                    close(pipe_pip);
                
                    insere_pendentes ( pendentes, nr_pendentes, args, i);
                    nr_pendentes++;
                }
            }

            if(flag && fork()==0){                                         // executamos o pedido caso seja possível 
                close (pipe_read);
                write( pipe_pip, "...executing...\n", 16);
                
                int p[i-1][2];
                int j;
                
                int f_entrada = open (ficheiro_entrada, O_RDONLY);
                int f_saida = open (ficheiro_saida, O_WRONLY | O_CREAT | O_TRUNC, 0640);
                
                //int fd_aux = dup(1);                                       // guarda no descritor fd_aux o descritor do  
                
                dup2(f_entrada,0);                                         // guarda a referencia do descritor do ficheiro de entrada no descritor 0
                dup2(f_saida,1);                                           // guarda a referencia do descritor do ficheiro de saida no descritor 1
                close(f_saida);
                close(f_entrada);
                
                char*cmd=malloc(30);
                strcpy(cmd,argv[2]);                                       // guarda num novo array o segundo argumento, isto é, o caminho para os executáveis   
                
                for (j=0; j<i; j++){                                       // i neste ponto corresponde ao nº de transformações-1 (começa em 0) que um pedido tem
                    if (j==0){                                             // se for o 1º pipe, isto é, aquele que apenas vai conseguir escrever     
                        pipe(p[j]);                                        // cria o pipe
                        if (fork()==0){                                     
                            close (pipe_write);
                            close(pipe_pip);
                            close(p[j][0]);                                // fecha o de leitura   ???????
                            dup2(p[j][1],1);
                            close(p[j][1]);
                            strcat(cmd,args[j]);
                            execlp(cmd, args[j], NULL);                    // executa o pedido 
                            _exit(0);
                        }
                        else close(p[j][1]);
                    }
                    else if (j==i-1){                                      // se for o último pipe, isto é, aquele que apenas vai conseguir ler
                        if (fork()==0){
                            close (pipe_write);
                            close(pipe_pip);
                            dup2(p[j-1][0],0);
                            close(p[j-1][0]);
                            strcat(cmd,args[j]);
                            execlp(cmd, args[j], NULL);
                            _exit(0);
                        }
                        else close(p[j-1][0]);
                    }
                    else {                                                 // se for um dos pipes do meio que tanto conseguem ler como escrever
                        pipe(p[j]);
                        if (fork()==0){
                            close (pipe_write);
                            close(pipe_pip);
                            close(p[j][0]);
                            dup2(p[j][1],1);
                            close(p[j][1]);
                            dup2(p[j-1][0],0);
                            close(p[j-1][0]);
                            strcat(cmd,args[j]);
                            execlp(cmd, args[j], NULL);
                            _exit(0);
                        }
                        else {
                            close(p[j][1]);
                            close(p[j-1][0]);
                        }
                    }
                }

                for (j=0; j<i; j++) wait(NULL);                           // espera que os processos terminem  MÁ PRÁTICA USAR O NULL           
                write (pipe_pip, "...finished...\n", 15);                 // informa o cliente que o pedido foi executado
                
                char * b = malloc (50);
                sprintf (b, "bytes-input: %d  bytes-output: %d \n", bytes(ficheiro_entrada), bytes(ficheiro_saida) );
                write (pipe_pip, b, strlen(b) );

                // aviso é um pedido especial, manda um pedido ao servidor a avisar que já terminou o outro pedido e assim voltar a pôr direito os valores no array limites 
                // e a voltar a analisar os pendentes para ver se já podemos executar algum
                char * aviso = malloc(100);
                strcpy(aviso,pid);
                strcat(aviso," Done ");
                for (int k=0; k<i;k++){
                    strcat(aviso,args[k]);
                    strcat(aviso," ");
                }
                strcat(aviso,"\n");
                write(pipe_write,aviso,strlen(aviso));                    // irá ter o pid do cliente, Done e os args das transformações executadas  
                close (pipe_write);
                close(pipe_pip);
                _exit(0);
                
            }
            close(pipe_pip);
        }

        else if (!strcmp(pedido, "Done")){
        close(pipe_pip);
            while (buf){                                                  // uma vez que o pedido foi executado voltamos a incrementar os limites a indicar transformações disponiveis
               char * op =  strdup(strsep((&buf)," \n"));
                if (!strcmp(op, "nop")) limites[0]++;
                else if (!strcmp(op, "bcompress")) limites[1]++;
                else if (!strcmp(op, "bdecompress")) limites[2]++;
                else if (!strcmp(op, "gcompress")) limites[3]++;
                else if (!strcmp(op, "gdecompress")) limites[4]++;
                else if (!strcmp(op, "encrypt")) limites[5]++;
                else if (!strcmp(op, "decrypt")) limites[6]++;
            }

                int aux[7];                                               // criar um novo array como cópia do array limites para ser manipulado à medida que analisamos
                for (i=0; i<7; i++) aux[i] = limites[i];                  // a matriz com os pendentes.

                for(i=0; i<nr_pendentes ; i++){                           // comecamos a analisar os pendentes
                    flag = 1;
    
                    for (j=0; pendentes[i][j] != NULL && flag; j++) {

                         if (!strcmp(pendentes[i][j], "nop") && aux[0]>0) aux[0]--;
                         else if (!strcmp(pendentes[i][j], "bcompress") && aux[1]>0) aux[1]--;
                         else if (!strcmp(pendentes[i][j], "bdecompress") && aux[2]>0) aux[2]--;
                         else if (!strcmp(pendentes[i][j], "gcompress") && aux[3]>0) aux[3]--;
                         else if (!strcmp(pendentes[i][j], "gdecompress") && aux[4]>0) aux[4]--;
                         else if (!strcmp(pendentes[i][j], "encrypt") && aux[5]>0) aux[5]--;
                         else if (!strcmp(pendentes[i][j], "decrypt") && aux[6]>0) aux[6]--;
                         else {                                          // o pedido continua pendente
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

                    if (flag){                                           // o pedido pode ser executado                                  
                        //criar o pedido
                        char *pedido_novo = malloc(2000);
                        strcpy (pedido_novo, pendentes[i][j+3]);
                        strcat (pedido_novo, " proc-file ");
                        strcat (pedido_novo, pendentes[i][j+1]);
                        strcat (pedido_novo, " ");
                        strcat (pedido_novo, pendentes[i][j+2]);
                        strcat (pedido_novo, " ");

                        for (int c = 0; c < j; c++){  
                            strcat(pedido_novo,pendentes[i][c]);
                            strcat(pedido_novo, " ");
                        }
                        strcat(pedido_novo, "\n");

                        write(pipe_write, pedido_novo, strlen(pedido_novo));
                        char ** temp = pendentes[0];
                        retira_pedido (&temp, i--, nr_pendentes--); // é retirado dos pendentes

                    }

                }
                
            }
    }
}
