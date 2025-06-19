#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFERSIZE 1024
#define SERVERIP "71.58.245.69"
#define PORT 7777


int main(){
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0){
        perror("socket");
        exit(1);
    }
    struct sockaddr_in server;

    server.sin_family = AF_INET;
    server.sin_port   = htons(PORT);
    inet_pton(AF_INET, SERVERIP, &server.sin_addr);

    if(connect(sockfd, (struct sockaddr*)&server, sizeof(server)) < 0){
        perror("connect failed");
        exit(1);
    }

    char cmd[64];
    char buffer[BUFFERSIZE];
    int bytesReceived;
    while (1){
        printf("Enter a command:\n  LIST\n  DOWNLOAD\n  UPLOAD\n  DELETE\n  RENAME\n  EXIT\n\n->");
        if (!fgets(cmd, sizeof(cmd), stdin)){
            break;
        } 
        cmd[strcspn(cmd, "\r\n")] = '\0'; // strips off \r\n and replaces w \0

        if (strcmp(cmd, "LIST") == 0){
            send(sockfd, "LIST", strlen("LIST"), 0);

            while((bytesReceived = recv(sockfd, buffer, sizeof(buffer) - 1, 0)) > 0){
                buffer[bytesReceived] = '\0';
                printf("%s", buffer);
                if (strstr(buffer, "END") != NULL){ // if END is in the buffer were done
                    break;
                }
            }
        }else if (strcmp(cmd, "UPLOAD") == 0){
           printf("not done UPLOAD yet");
        }else if (strcmp(cmd, "DOWNLOAD") == 0){
            printf("not done DOWNLOAD yet");
        }
        else if (strcmp(cmd, "EXIT") == 0){
            send(sockfd, "EXIT", 4, 0);
            break;
        }
        else{
            printf("not a cmd\n");
        }
    }
    close(sockfd);
    return 0;
}
