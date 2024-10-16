#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h> // Import for `errno` variable

#include <fcntl.h>      // Import for `fcntl` functions
#include <sys/types.h>  // Import for `socket`, `bind`, `listen`, `accept`, `fork`, `lseek` functions
#include <sys/socket.h> // Import for `socket`, `bind`, `listen`, `accept` functions
#include <netinet/ip.h>
#include <ctype.h>

#define MAX_KEYS 100
#define BUF_SIZE 8000

struct KeyValue {
    char key[50];
    char value[100];
};

struct KeyValue storage[MAX_KEYS];
int storage_count = 0;

void trim(char *str) {
    char *end;
    while(isspace((unsigned char)*str)) str++;
    if(*str == 0) return;
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
}

char* get_value(const char* key) {
    for (int i = 0; i < storage_count; i++) {
        if (strcmp(storage[i].key, key) == 0) {
            return storage[i].value;
        }
    }
    return NULL;
}

int put_value(const char* key, const char* value) {
    for (int i = 0; i < storage_count; i++) {
        if (strcmp(storage[i].key, key) == 0) {
            strcpy(storage[i].value, value);
            return 1;
        }
    }
    if (storage_count < MAX_KEYS) {
        strcpy(storage[storage_count].key, key);
        strcpy(storage[storage_count].value, value);
        storage_count++;
        return 1;
    }
    return 0;
}

int delete_value(const char* key) {
    for (int i = 0; i < storage_count; i++) {
        if (strcmp(storage[i].key, key) == 0) {
            for (int j = i; j < storage_count - 1; j++) {
                storage[j] = storage[j + 1];
            }
            storage_count--;
            return 1;
        }
    }
    return 0;
}

void clientHandler(int nsd) {
    size_t readBytes;
    char buf[BUF_SIZE];
    char response[BUF_SIZE];
    do {
        memset(buf, 0, sizeof(buf));
        readBytes = read(nsd, buf, sizeof(buf) - 1);
        if (readBytes > 0) {
            printf("Message from client: %s\n", buf);
            trim(buf);

            char command[10], key[50], value[100];
            if (sscanf(buf, "%s %s %[^\n]", command, key, value) >= 2) {
                if (strcasecmp(command, "GET") == 0) {
                    char* result = get_value(key);
                    if (result) {
                        snprintf(response, BUF_SIZE, "Value: %s\n", result);
                    } else {
                        snprintf(response, BUF_SIZE, "Key not found\n");
                    }
                } else if (strcasecmp(command, "PUT") == 0) {
                    if (put_value(key, value)) {
                        snprintf(response, BUF_SIZE, "Value stored successfully\n");
                    } else {
                        snprintf(response, BUF_SIZE, "Storage full\n");
                    }
                } else if (strcasecmp(command, "DELETE") == 0) {
                    if (delete_value(key)) {
                        snprintf(response, BUF_SIZE, "Key deleted successfully\n");
                    } else {
                        snprintf(response, BUF_SIZE, "Key not found\n");
                    }
                } else {
                    snprintf(response, BUF_SIZE, "Invalid command\n");
                }
            } else {
                snprintf(response, BUF_SIZE, "Invalid input format\n");
            }
            write(nsd, response, strlen(response));
        }
    } while(readBytes > 0);
}

int main(){
    struct sockaddr_in serv,cli;
    int sd,nsd,sz;
    
    sd = socket(AF_INET,SOCK_STREAM,0);
    if (sd == -1)
    {
        perror("Error while creating server socket!");
        _exit(0);
    }
    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = INADDR_ANY;
    serv.sin_port = htons(8081);
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