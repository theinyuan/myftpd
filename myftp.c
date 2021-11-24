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
int retrieveDataSocket(int *socketNum);

// main begins here
int main(int argc, char *argv[])
{
    startClientProg(argc, argv);
    return 0;
}

// functions implementation
int startClientProg(int argumentCount, char *argumentValue[])
{
    int cliConnSocket, socketStatus, nr, nw, i;
    char input[MAX_BLOCK_SIZE], host[60];

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
    socketStatus = initClientProg("russell-VirtualBox", &cliConnSocket);
    if(socketStatus != 0)
    {
        printf("Failed to connect to the server. Terminating Program.\n");
        exit(socketStatus); 
    }

    do
    {
        printf("> ");
        fgets(input, sizeof(input), stdin); // reading user input and storing into input
        nr = strlen(input);
        if(input[nr-1] == '\n')
        {
            input[nr-1] = '\0';
            nr--;
        }

        if(nr > 0)
        {
            if((nw = writeContent(socketStatus, input, nr)) < nr)
            {
                printf("Client: Send Error\n");
                exit(1);
            }

            if((nr = readContent(socketStatus, input, sizeof(input))) <= 0)
            {
                printf("Client: Receive Error\n");
                exit(1);
            }

            input[nr] ='\0';
            i++;
            printf("Server Output[%d]: %s\n", i, input);
        }

    } while (strcmp(input, "quit") != 0);

    close(cliConnSocket);

    printf("You are now exiting the client. Goodbye!\n");
    return(socketStatus);
}

int initClientProg(char *serverName, int *socketNum)
{
    int sock;
    struct sockaddr_in clientAddr;
    struct sockaddr_in serverAddr;
    struct hostent *hostport;

    if((hostport = gethostbyname(serverName)) == NULL)
    {
        printf("host %s not found.\n", serverName);
        return(INVALID_HOST_NAME);
    }

    if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Failed to create socket");
        return(CREATE_SOCKET_FAILED);
    }

    bzero((char *)&clientAddr, sizeof(clientAddr));

    clientAddr.sin_family = AF_INET;
    clientAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    clientAddr.sin_port = 0;

    if(bind(sock,(struct sockaddr *)&clientAddr, sizeof(clientAddr)) < 0)
    {
        perror("Unable to bind");
        return(BIND_FAILED);
    }

    bzero((char *)&serverAddr, sizeof(serverAddr));

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = * (u_long *) hostport->h_addr;
    serverAddr.sin_port = htons(SERVER_FTP_PORT);

    if(connect(sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        perror("Failed to connect to server");
        return(CONNECT_FAILED);
    }

    *socketNum = sock;
    
    return(OK);
}

int retrieveDataSocket(int *socketNum)
{
    int sock;
    struct sockaddr_in serverAddr;

    if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Failed to create socket");
        return(CREATE_SOCKET_FAILED);
    }

    bzero((char *)&serverAddr, sizeof(serverAddr));

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(SERVER_FTP_PORT);

    if(bind(sock,(struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        perror("Unable to bind socket to server IP");
        return(BIND_FAILED);
    }

    listen(sock, serverQueue);

    *socketNum = sock;

    return(OK);
}