#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <fcntl.h> 
#include <string.h>
#include <stdlib.h>


int main(int argc, char *argv[]){
    // Open pipe
    int pipe_write = open("pipe", O_WRONLY); 
    char *pid = malloc(15);
    sprintf(pid,"%d", (int) getpid());
    if(mkfifo(pid, 0666)==-1) perror ("fifo_cliente");
    char buf[1024];

    if(pipe_write < 0) {
        perror("Couldn't open the pipe!");
        return 1;
    }

    char *pedido = malloc(2000);
    strcpy (pedido, pid);

    if(!strcmp("status",argv[1])) {
        strcat(pedido, " status\n");
    }
    else if (!strcmp("proc-file",argv[1])){

        strcat(pedido, " proc-file ");   
        for (int i=2; i<argc; i++){
            strcat(pedido,argv[i]);
            strcat(pedido, " ");
        }
        strcat(pedido,"\n");
    }
    write (pipe_write, pedido, strlen(pedido));
    close(pipe_write);
    int pipe_read = open(pid, O_RDONLY);
    pipe_write = open(pid, O_WRONLY);
    int read_bytes;  

    while((read_bytes = read(pipe_read,buf,1024))>0){
        write(1,buf,read_bytes);
        buf[read_bytes] = '\0';
        if ((!strcmp (buf, "...finished...\n")) || (!strcmp(argv[1], "status")))
            close(pipe_write);
    }
    close(pipe_read);
    unlink(pid);
    
    return 0;
}