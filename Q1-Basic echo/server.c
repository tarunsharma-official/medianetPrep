#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h> // Import for `errno` variable

#include <fcntl.h>      // Import for `fcntl` functions
#include <sys/types.h>  // Import for `socket`, `bind`, `listen`, `accept`, `fork`, `lseek` functions
#include <sys/socket.h> // Import for `socket`, `bind`, `listen`, `accept` functions
#include <netinet/ip.h>

void clientHandler(int nsd){
    size_t readBytes;
    char buf[8000];
    do{
        memset(buf, 0, sizeof(buf));
        readBytes = read(nsd, buf, sizeof(buf) - 1);
        if (readBytes > 0) {
            printf("Message from client: %s\n", buf);
            write(nsd, buf, sizeof(buf));
        }
    }while(readBytes>0);
}

int main(){
    struct sockaddr_in serv,cli;
    int sd,nsd,sz;
    
    sd = socket(AF_UNIX,SOCK_STREAM,0);
    if (sd == -1)
    {
        perror("Error while creating server socket!");
        _exit(0);
    }
    serv.sin_family = AF_UNIX;
    serv.sin_addr.s_addr = INADDR_ANY;
    serv.sin_port = htons(8080);
    int socketBindStatus = bind(sd,(struct sockaddr *) (&serv),sizeof(serv));
    if (socketBindStatus == -1)
    {
        perror("Error while binding to server socket!");
        _exit(0);
    }
    int socketListenStatus =listen(sd,5);
    if (socketListenStatus == -1)
    {
        perror("Error while listening for connections on the server socket!");
        close(sd);
        _exit(0);
    }
    sz=sizeof(cli);
    
    while(1){
        nsd=accept(sd,(struct sockaddr *)(&cli),&sz);
        if (nsd == -1) {
            perror("Error accepting connection");
            continue;
        }
        if(!fork()){
            close(sd);
            clientHandler(nsd);
            exit(0);
        }
     
     
    }
    close(nsd);
}