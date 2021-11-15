/*
 *  File: myftpd.c (server program)
 *  Authors: Russell and TY
 *  Date Start: 12 Nov 2021
 *  Purpose: Project 2 - A Simple File Transfer Protocol
 *  Description: a simple network protocol that can be used to download files from a remote site and to upload files to a remote site, and a client and a server programs that communicate using that protocol.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include "protocol.h"

int daemon_init();
int startServerProg();
int initServerProg(int *socketNum);

char *accessLog = "svr_access_log.txt";
char *errorLog = "svr_error_log.txt";
char programTime[MAX_TIME] = "";

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
        exit(OK);
    }

    setsid();
    chdir("/");
    umask(0);
    return OK;
}

int startServerProg()
{
    int socketStatus, messageSize, serverSocket, cliConnSocket;
    currentTime(programTime);
    svrAccessLog = fopen(accessLog, "a");
    svrErrorLog = fopen(errorLog, "a");

    if (svrAccessLog == NULL)
    {
        printf("Error: can't create log file %s\n", accessLog);
    }
    if (svrErrorLog == NULL)
    {
        printf("Error: can't create log file %s\n", errorLog);
    }

    if (daemon_init() < 0)
    {
        printf("Unable to convert into daemon\n"),exit(4);
    }

    fprintf(svrAccessLog,"%s server pid = %d\n",programTime,getpid());
    fflush(svrAccessLog);

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
        return ACCEPT_FAILED;
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