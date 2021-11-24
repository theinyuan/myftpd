/*
 *  File: myftp.c (client program)
 *  Authors: Russell and TY
 *  Date Start: 12 Nov 2021
 *  Purpose: Project 2 - A Simple File Transfer Protocol
 *  Description: a simple network protocol that can be used to download files from a remote site and to upload files to a remote site, and a client and a server programs that communicate using that protocol.
 */

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>

#include "protocol.h"

// functions declarations
int startClientProg(int argumentCount, char *argumentValue[]);
int initClientProg(char *serverName, int *socketNum);
int sendInput(int socket, char *input, int nBytes);

// main begins here
int main(int argc, char *argv[])
{
    startClientProg(argc, argv);
    return 0;
}

// functions implementation
int startClientProg(int argumentCount, char *argumentValue[])
{
    int cliConnSocket, socketStatus, sd, nr, nw, i = 0;
    char input[MAX_BLOCK_SIZE], cmd[MAX_BLOCK_SIZE], arg[MAX_BLOCK_SIZE], host[60];

    if(argumentCount == 1)
    {
        gethostname(host, sizeof(host));
    }
    else if(argumentCount == 2)
    {
        strcpy(host, argumentValue[1]);
    }
    else
    {
        printf("Usage: %s [<server_host_name>]\n", argumentValue[0]);
        exit(1);
    }    

    // starting myftp
    socketStatus = initClientProg(host, &sd);
    if(socketStatus != 0)
    {
        printf("Failed to connect to the server. Terminating Program.\n");
        exit(socketStatus); 
    }

    do
    {
        printf("> ");
        fgets(input, sizeof(input), stdin);
        nr = strlen(input);
        if(input[nr-1] == '\n')
        {
            input[nr-1] = '\0'; // stripping newline
        }

        if(strstr(input, " ") != NULL)
        {
            strcpy(cmd, strtok(input, " "));
            strcpy(arg, strtok(NULL, " "));
            if(strcmp(cmd, "put") == 0)
            {
                FILE *aFile;
                char buffer[MAX_BLOCK_SIZE];
                int nBytes = 0;
                int sd;

                if(socketStatus != OK)
                {
                    // printf("");
                }
                sd = accept(sd, NULL, NULL);
                aFile = fopen(arg, "r");
                if(aFile != NULL){
                    while(!feof(aFile))
                    {
                        nBytes = fread(buffer, sizeof(char), MAX_BLOCK_SIZE, aFile);
                        socketStatus = writeContent(sd, buffer, strlen(buffer)+1);
                        if(socketStatus != OK)
                        {
                            break;
                        } // end of if
                    } // end of whole
                    close(sd);
                    fclose(aFile);
                } // end of if
                else
                {
                    printf("File does not exist\n");
                    close(sd);
                }
            }
            else if(strcmp(cmd, "get") == 0)
            {
                // insert code here
            }
        }
    } while (strcmp(input, "quit") != 0);

    close(cliConnSocket);

    printf("You are now exiting the client. Goodbye!\n");
    exit(socketStatus);
}

int initClientProg(char *serverName, int *socketNum)
{
    int sock;
    struct sockaddr_in serverAddr;
    struct hostent *hp;

    if((hp = gethostbyname(serverName)) == NULL)
    {
        printf("host %s not found.\n", serverName);
        return(INVALID_HOST_NAME);
    }

    if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Failed to create socket");
        return(CREATE_SOCKET_FAILED);
    }

    bzero((char *)&serverAddr, sizeof(serverAddr));

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_FTP_PORT);
    serverAddr.sin_addr.s_addr = * (u_long *) hp->h_addr;

    if(connect(sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        perror("Failed to connect to server");
        return(CONNECT_FAILED);
    }

    *socketNum = sock;
    
    return(OK);
}

int sendInput(int socket, char *input, int nBytes)
{
    int i;

    if((send(socket, input, nBytes, 0)) < 0)
    {
        perror("Client: Send failure");
        exit(1);
    }

    return(OK);
}