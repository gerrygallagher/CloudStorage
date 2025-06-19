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
    int socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if(socketfd == -1){
        perror("socket creation failed");
        close(socketfd);
        exit(1);
    }
    printf("A socket was created!\n");

    // contains the info the server(this file) needs to bind to a port
    struct sockaddr_in serverAddress;

    // set the data for the server
    serverAddress.sin_family = AF_INET; // using IPv4
    serverAddress.sin_port = htons(PORT); // set port number, converted to network byte order
    serverAddress.sin_addr.s_addr = INADDR_ANY; // can accept connections from any port
    memset(serverAddress.sin_zero, 0, sizeof(serverAddress.sin_zero)); // set padding to 0

    //added this so i can rerun it right away
    int opt = 1;
    setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

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



    while(1){
        // accept a connection when someone tries to connect
        struct sockaddr_in clientAddress; // stores the clients info
        socklen_t clientAddressLength  = sizeof(clientAddress); // to hold the length the client gives 
        int clientfd = accept(socketfd, (struct sockaddr*)&clientAddress, &clientAddressLength);
        if( clientfd == -1){
            perror("failed to accpet");
            close(socketfd);
            exit(1);
        }
        printf("Client accepted on port %d client socket number: %d\n", PORT, clientfd);



        char buffer[BUFFERSIZE]; // buffer to receive client commands
        char filename[BUFFERSIZE]; // buffer the receive client filename
        int bytesReceived;
        while(1){
            // get command from client 
            bytesReceived = recv(clientfd, buffer, sizeof(buffer)-1, 0); // blocking operation
            if(bytesReceived < 0){ // recv failed
                perror("receive failed");
                break;
            }
            if(bytesReceived == 0){ // client closed connection
                printf("Client disconnected\n");
                break;
            }
            // At this point bytesReceived > 0
            buffer[bytesReceived] = '\0';              // null terminate what the client sent
            // receive sucessful!

        
            // COMMANDS



            if(strncmp(buffer, "LIST", 4) == 0){              // LIST
                // open the storage directory
                DIR *directory = opendir("storage");
                if(directory == NULL){
                    send(clientfd, "ERROR: cannot open storage\n", strlen("ERROR: cannot open storage\n"), 0);
                    continue;
                }

                struct dirent *entry;
                while((entry = readdir(directory)) != NULL){ // loop over all files in the storage directory
                    if(entry->d_type == DT_REG){ // files only not folders or other things
                        send(clientfd, entry->d_name, strlen(entry->d_name), 0); // send the file name
                        send(clientfd, "\n", 1, 0); // send a newline so the list looks good
                    }
                }

                //close directory
                closedir(directory);
                send(clientfd, "END\n", 4, 0);
                continue;
            }else if(strncmp(buffer, "UPLOAD", 6) == 0){      // UPLOAD
                // ask for filename
                send(clientfd, "Enter filename to upload:\n", strlen("Enter filename to upload:\n"), 0);

                // recv filename
                bytesReceived = recv(clientfd, filename, sizeof(filename) - 1, 0);
                if(bytesReceived <= 0){
                    send(clientfd, "ERROR(UPLOAD): didn't recv filename\n", strlen("ERROR(UPLOAD): didn't recv filename\n"), 0);
                    continue;
                }
                filename[bytesReceived] = '\0'; // null terminate filename

                // acknowledge
                send(clientfd, "FILENAME RECEIVED\n", strlen("FILENAME RECEIVED\n"), 0);

                // create and open file to write to
                char path[BUFFERSIZE]; 
                snprintf(path, sizeof(path), "storage/%s", filename); // make a file called '%s', filename
                FILE *filePointer = fopen(path, "wb");
                if(filePointer == NULL){ 
                    send(clientfd, "ERROR(UPLOAD): couldnt create file\n", strlen("ERROR(UPLOAD): couldnt create file\n"), 0);
                    continue;
                }

                // recv file data
                while(1){
                    bytesReceived = recv(clientfd, buffer, BUFFERSIZE - 1, 0);
                    if(bytesReceived <= 0){
                        break;
                    }
                    buffer[bytesReceived] = '\0'; // null terminate 

                    // check for end marker
                    char *endMarker = strstr(buffer, "__END__");
                    if(endMarker){
                        fwrite(buffer, 1, endMarker - buffer, filePointer);
                        break;
                    }
                    // write the data to the file
                    fwrite(buffer, 1, bytesReceived, filePointer);
                }
                fclose(filePointer);

                // ack
                send(clientfd, "UPLOAD SUCCESS\n", strlen("UPLOAD SUCCESS\n"), 0);
                printf("File Uploaded: %s", filename);
                continue;

            }else if(strncmp(buffer, "DOWNLOAD", 8) == 0){    // DOWNLOAD
                // ask for filename
                send(clientfd, "Enter filename to download\n->", strlen("Enter filename to download\n->"), 0);

                //recv file name from client
                bytesReceived = recv(clientfd, filename, sizeof(filename) - 1, 0);
                buffer[bytesReceived] = '\0';

                // open file if it exists
                char path[BUFFERSIZE];
                snprintf(path, sizeof(path), "storage/%s", filename);
                FILE *fp = fopen(path, "rb");
                if(!fp){
                    send(clientfd, "ERROR(DOWNLOAD): cannot open file\n", strlen("ERROR(DOWNLOAD): cannot open file\n"), 0);
                    continue;
                }

                // send file bit by bit
                while((bytesReceived = fread(buffer, 1, BUFFERSIZE - 1, fp)) > 0){
                    send(clientfd, buffer, bytesReceived, 0);
                }
                fclose(fp);

                // send end so client knows thats it
                send(clientfd, "__END__", strlen("__END__"), 0);
                continue;
            }else if(strncmp(buffer, "DELETE", 6) == 0){      // DELETE
                // ask for filename
                send(clientfd, "Enter filename to delete\n->", strlen("Enter filename to delete\n->"), 0);

                //recv file name from client
                bytesReceived = recv(clientfd, filename, sizeof(filename) - 1, 0);
                buffer[bytesReceived] = '\0';

                //make path to file
                char path[BUFFERSIZE];
                snprintf(path, sizeof(path), "storage/%s", filename);
                
                // remove file and send success msg or errir message
                if(remove(path) == 0){
                    send(clientfd, "DELETE SUCCESS\n", strlen("DELETE SUCCESS\n"), 0);
                }else{
                    send(clientfd, "ERROR(DELETE): failed to delete\n", strlen("ERROR(DELETE): failed to delete\n"), 0);
                }
                continue;
            }else if(strncmp(buffer, "RENAME", 6) == 0){      // RENAME
                send(clientfd, "RENAME command not implemented yet\n", strlen("RENAME command not implemented yet\n"), 0);
            }else if(strncmp(buffer, "EXIT", 4) == 0){        // EXIT
                send(clientfd, "now exiting, thanks for using gerry's cloud storage server\n", strlen("now exiting, thanks for using gerry's cloud storage server\n"), 0);
                break;
            }else{

            }
        }
        close(clientfd);
        printf("Client disconnected, waiting for next client...\n");
    }    
    close(socketfd);
    return 0;
}