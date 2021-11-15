/*
 *  Authors: Russell and TY
 *  Date Start: 12 Nov 2021
 *  Purpose: Server program
 */

#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include "protocol.h"

int daemon_init();
void startServerProg();
int initServerProg(int *socketNum);

int main()
{
    startServerProg();
    return 0;
}

int daemon_init()
{
    pid_t pid;
    if ((pid = fork()) < 0)
    {
        perror("fork error");
        exit(FORK_ERROR);
    }
    else if (pid != 0)
    {
        exit(OK); // parent goes bye-bye
    }
    // child continues
    setsid();   // become session leader
    chdir("/"); // change working directory
    umask(0);   // clear our file mode creation mask
    return OK;
}

void currentTime(char *timeNow)
{
    int hours,minutes,seconds,day,month,year;
    time_t now = time(NULL);
    struct tm *local = localtime(&now);
    strftime(timeNow,sizeof(timeNow),"%Y:%m:%d:%h:%M:%S",&local);
}

void startServerProg()
{
    int socketStatus, messageSize, serverSocket, cliConnSocket;
    currentTime(timeNow);
    accessLogFile = fopen(accessLog,"w");
    errorLogFile = fopen(errorLog,"w");

    if(daemon_init() < 0)
    {

    }

    printf("Initialise server FTP program...\n");
    socketStatus = initServerProg(&serverSocket);
    if (socketStatus != 0)
    {
        printf("Socket error. This program will be terminated.\n");
        exit(socketStatus);
    }

    printf("Socket created successfully and ready to accept connection from clients.\n");
    cliConnSocket = accept(serverSocket, NULL, NULL);
    if (cliConnSocket < 0)
    {
        perror("Error in accepting connection from clients: ");
        printf("Program is terminating...\n");
        close(serverSocket);
        return (ACCEPT_FAILED);
    }
    return OK;
}

int initServerProg(int *socketNum)
{
    int sock;
    struct sockaddr_in serviceAddr;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Failed to create socket");
        return (CREATE_SOCKET_FAILED);
    }

    memset((char *)&serviceAddr, 0, sizeof(serviceAddr));

    serviceAddr.sin_family = AF_INET;
    serviceAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serviceAddr.sin_port = htons(SERVER_FTP_PORT);

    if (bind(sock, (struct sockaddr *)&serviceAddr, sizeof(serviceAddr)) < 0)
    {
        perror("Unable to bind socket to the server IP");
        close(sock);
        return (BIND_FAILED);
    }

    listen(sock, serverQueue);
    *socketNum = sock;
    return (OK);
}