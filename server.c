#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <dirent.h>



#define PORT 7777
#define BACKLOG 1 // will only accept 1 connection at a time
#define BUFFERSIZE 1024


int main(){
    //create a socket file descriptor
    int socketfd = socket(AF_INET, SOCK_STREAM, 0);\
    if(socketfd == -1){
        perror("socket creation failed");
        close(socketfd);
        exit(1);
    }
    printf("A socket was created!");

    // contains the info the server(this file) needs to bind to a port
    struct sockaddr_in serverAddress;

    // set the data for the server
    serverAddress.sin_family = AF_INET; // using IPv4
    serverAddress.sin_port = htons(PORT); // set port number, converted to network byte order
    serverAddress.sin_addr.s_addr = INADDR_ANY; // can accept connections from any port
    memset(serverAddress.sin_zero, 0, sizeof(serverAddress.sin_zero)); // set padding to 0

    // bind the socket to the port
    if(bind(socketfd, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) == -1){
        perror("binding failed");
        close(socketfd);
        exit(1);
    }
    printf("Socket bound to port number: %d\n", PORT);
    
    // begin listening on the port for connections
    if(listen(socketfd, BACKLOG) == -1){
        perror("listening failed");
        close(socketfd);
        exit(1);
    }
    printf("Server is now listening on port number: %d\n", PORT);

    // accept a connection when someone tries to connect
    struct sockaddr_in clientAddress; // stores the clients info
    socklen_t clientAddressLength  = sizeof(clientAddress); // to hold the length the client gives 
    int clientfd = accept(socketfd, &clientAddress, &clientAddressLength);
    if( clientfd == -1){
        perror("failed to accpet");
        close(socketfd);
        exit(1);
    }
    printf("Client accepted on port %d client socket number: %d\n", PORT, clientfd);



    char buffer[BUFFERSIZE]; // buffer to receive client commands
    while(1){
        // send client instructions to enter
        const char *prompt = "Enter a command:\n  LIST\n  DOWNLOAD\n  UPLOAD\n  DELETE\n  RENAME\n  EXIT\n\n->";
        send(clientfd, prompt, strlen(prompt), 0);

        // get command from client 
        int bytesReceived = recv(clientfd, buffer, sizeof(buffer)-1, 0); // blocking operation
        if(bytesReceived == -1){
            perror("receive failed");
        }else if(bytesReceived > 0){
            buffer[bytesReceived] = '\0'; // null terminate what the client sent
        }else{
            printf("client was disconnected");
            break;
        }
        // receive sucessful!

  
        if(strncmp(buffer, "LIST", 4) == 0){              // LIST
            DIR *directory = opendir("storage");
            if (directory == NULL) {
                send(clientfd, "ERROR: cannot open storage\n", strlen("ERROR: cannot open storage\n"), 0);
            }
            // TO DO
            closedir(directory);
            send(clientfd, "END\n", 4, 0);
            continue;
        }else if(strncmp(buffer, "UPLOAD", 6) == 0){      // UPLOAD

        }else if(strncmp(buffer, "DOWNLOAD", 8) == 0){    // DOWNLOAD

        }else if(strncmp(buffer, "DELETE", 6) == 0){      // DELETE

        }else if(strncmp(buffer, "RENAME", 6) == 0){      // RENAME

        }else if(strncmp(buffer, "EXIT", 4) == 0){        // EXIT

        }else{

        }
    }
    close(socketfd);
}