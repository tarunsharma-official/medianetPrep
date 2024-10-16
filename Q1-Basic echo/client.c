#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h> // Import for `errno` variable

#include <fcntl.h>      // Import for `fcntl` functions
#include <sys/types.h>  // Import for `socket`, `bind`, `listen`, `accept`, `fork`, `lseek` functions
#include <sys/socket.h> // Import for `socket`, `bind`, `listen`, `accept` functions
#include <netinet/ip.h>

int main() {
    struct sockaddr_in serv;
    int sd;
    char readbuf[8000], writebuf[8000];
    ssize_t readBytes, writeBytes;
    
    sd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sd == -1) {
        perror("Error creating socket");
        exit(1);
    }
    
    serv.sin_family = AF_UNIX;
    serv.sin_addr.s_addr = INADDR_ANY;
    serv.sin_port = htons(8080);  // Make sure this matches the server port
    
    if (connect(sd, (struct sockaddr *)&serv, sizeof(serv)) == -1) {
        perror("Error connecting to server");
        close(sd);
        exit(1);
    }
    
    do {
        memset(writebuf, 0, sizeof(writebuf));
        memset(readbuf, 0, sizeof(readbuf));
        
        printf("Enter a message that you want to send (or press Enter to quit): ");
        if (fgets(writebuf, sizeof(writebuf), stdin) == NULL) {
            break;
        }
        
        if (writebuf[0] == '\n') {
            break;
        }
        
        writeBytes = write(sd, writebuf, strlen(writebuf));
        if (writeBytes == -1) {
            perror("Error writing to socket");
            break;
        }
        
        readBytes = read(sd, readbuf, sizeof(readbuf) - 1);
        if (readBytes > 0) {
            printf("Message from server: %s", readbuf);
        } else if (readBytes == 0) {
            printf("Server closed the connection\n");
            break;
        } else {
            perror("Error reading from socket");
            break;
        }
    } while (1);
    
    close(sd);
    return 0;
}