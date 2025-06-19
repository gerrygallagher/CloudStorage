#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFERSIZE 1024
// #define SERVERIP "71.58.245.69"
#define SERVERIP "10.0.0.118"
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
    char filename[BUFFERSIZE];
    while (1){
        printf("Enter a command:\n  LIST\n  DOWNLOAD\n  UPLOAD\n  DELETE\n  RENAME\n  EXIT\n\n->");
        fflush(stdout);
        if (!fgets(cmd, sizeof(cmd), stdin)){
            break;
        } 
        cmd[strcspn(cmd, "\r\n")] = '\0'; // strips off \r\n and replaces w \0

        if (strcmp(cmd, "LIST") == 0){                // LIST
            send(sockfd, "LIST", strlen("LIST"), 0);
            printf("List of files:\n------------\n");
            while((bytesReceived = recv(sockfd, buffer, sizeof(buffer) - 1, 0)) > 0){
                buffer[bytesReceived] = '\0';
                
                char *endMarker = strstr(buffer, "END");
                if (endMarker != NULL) {
                    *endMarker = '\0';  // cut off END and anything after
                    printf("%s", buffer);
                    break;
                }

                printf("%s", buffer);
            }
            printf("------------\n\n\n");
            continue;
        }else if (strcmp(cmd, "UPLOAD") == 0){        // UPLOAD
            //send command
            send(sockfd, "UPLOAD", strlen("UPLOAD"), 0);

            // recv ask for filename
            bytesReceived = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
            buffer[bytesReceived] = '\0';
            printf("%s", buffer);

            //send filename
            if (!fgets(filename, sizeof(filename), stdin)){
                break;
            }
            filename[strcspn(filename, "\r\n")] = '\0';
            send(sockfd, filename, strlen(filename), 0);

            // recv ack
            bytesReceived = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
            buffer[bytesReceived] = '\0';
            printf("%s", buffer);

            // open file to send
            FILE *fp = fopen(filename, "rb");
            if(!fp){
                printf("Cannot open file '%s'\n", filename);
                return 1;
            }

            // send file contents
            while((bytesReceived = fread(buffer, 1, BUFFERSIZE - 1, fp)) > 0){
                send(sockfd, buffer, bytesReceived, 0);
            }
            fclose(fp);

            // send end marker
            send(sockfd, "__END__", strlen("__END__"), 0);

            // get last ack
            bytesReceived = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
            if (bytesReceived > 0) {
                buffer[bytesReceived] = '\0';
                printf("%s\n", buffer);  // should be "UPLOAD SUCCESS"
            }
            continue;
        }else if(strcmp(cmd, "DOWNLOAD") == 0){
            send(sockfd, "DOWNLOAD", 8, 0);

            //recv ask for file name
            bytesReceived = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
            buffer[bytesReceived] = '\0';
            printf("%s", buffer);

            //send filename
            if (!fgets(filename, sizeof(filename), stdin)){
                break;
            }
            filename[strcspn(filename, "\r\n")] = '\0';
            send(sockfd, filename, strlen(filename), 0);

                    
        // create file to write to
        FILE *fp = fopen(filename, "wb");
        if (!fp) {
            printf("Error creating local file\n");
            return 1;
        }

        // Receive and write file content
        while((bytesReceived = recv(sockfd, buffer, sizeof(buffer) - 1, 0)) > 0){
            buffer[bytesReceived] = '\0';

            // Check for end marker
            char *endMarker = strstr(buffer, "__END__");
            if(endMarker){
                fwrite(buffer, 1, endMarker - buffer, fp);
                break;
            }
            fwrite(buffer, 1, bytesReceived, fp);
        }
        fclose(fp);
        printf("Download complete for file: %s\n", filename);
        continue;
        }
        else if(strcmp(cmd, "EXIT") == 0){            // EXIT
            send(sockfd, "EXIT", 4, 0);
            bytesReceived = recv(sockfd, buffer, sizeof(buffer) -1, 0);
            buffer[bytesReceived] = '\0';
            printf("%s", buffer);
            break;
        }else if(strcmp(cmd, "RENAME") == 0){


        }else if(strcmp(cmd, "DELETE") == 0){
            // send command
            send(sockfd, "DELETE", 6, 0);

            // receive msg from server
            bytesReceived = recv(sockfd, buffer, sizeof(buffer) -1, 0);
            buffer[bytesReceived] = '\0';
            printf("%s", buffer);

            //send filename of file to delete
            if (!fgets(filename, sizeof(filename), stdin)){
                break;
            }
            filename[strcspn(filename, "\r\n")] = '\0';
            send(sockfd, filename, strlen(filename), 0);

            //recv a success or fail msg
            bytesReceived = recv(sockfd, buffer, sizeof(buffer) -1, 0);
            buffer[bytesReceived] = '\0';
            printf("%s", buffer);
            continue;
        }
        else{
            printf("not a cmd\n");
        }
    }
    close(sockfd);
    return 0;
}
