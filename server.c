#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>


#define PORT 7777
#define BACKLOG 1 // will only accept 1 connection at a time



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
    printf("Socket bound to port number: %d/n", PORT);
    
    // begin listening on the port for connections
    if(listen(socketfd, BACKLOG) == -1){
        perror("listening failed");
        close(socketfd);
        exit(1);
    }
    printf("Server is now listening on port number: %d\n", PORT);

    // accept a connection when someone tries to connect
    struct sockaddr_in clientAddress; // stores the clients info
    socklen_t clientAddressLength; // to hold the length the client gives 
    int clientfd = accept(socketfd, &clientAddress, &clientAddressLength);
    if( clientfd == -1){
        perror("faiedl to accpet");
        close(socketfd);
        exit(1);
    }
    printf("Client accepted on port %d client socket number: %d\n", PORT, clientfd);


}