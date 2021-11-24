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

#include "protocol.h"

// functions declarations
int startClientProg(int argumentCount, char *argumentValue[]);
int initClientProg(int *serverName, int *socketNum);

// main begins here
int main(int argc, char *argv[])
{
    startClientProg(argc, argv);
    return 0;
}

// functions implementation
int startClientProg(int argumentCount, char *argumentValue[])
{

}

int initClientProg(int *serverName, int *socketNum)
{
    int sock;
    struct sockaddr_in clientAddr;
    struct sockaddr_in serverAddr;
    struct hostent *hostport;

    if((hostport = gethostbyname(serverName) == NULL))
    {
        printf("host %s not found.\n", serverName);
        return(INVALID_HOST_NAME);
    }

    if((sock = socket(AF_INET, SOCK_STREAM, 0) < 0))
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


    listen(sock, serverQueue);
    *socketNum = sock;
    return(OK);
}