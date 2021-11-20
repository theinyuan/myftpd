/*
 *  File: myftpd.c (server program)
 *  Authors: Russell and TY
 *  Date Start: 12 Nov 2021
 *  Purpose: Project 2 - A Simple File Transfer Protocol
 *  Description: a simple network protocol that can be used to download files from a remote site and to upload files to a remote site, and a client and a server programs that communicate using that protocol.
 */

#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include "protocol.h"

void claim_children();
int daemon_init();
int startServerProg(int argumentCount, char *argumentValue[]);
int initServerProg(int *socketNum, char *initTime);
void serve_client(int socketNum, struct sockaddr *addr);
int svrSendMessage(int socketNum, char *bufMesg, int mesgSize, struct sockaddr *recipient, int recipientLeng, char *sendTime);
int svrRecvMessage(int socketNum, char *bufMesg, int bufferSize, int *mesgSize, struct sockaddr *sender, int *senderLeng, char *receiveTime);
void currentTime(char *timeNow);

FILE *svrAccessLog;
FILE *svrErrorLog;

int main(int argc, char *argv[])
{
    startServerProg(argc, argv);
    return 0;
}

void claim_children()
{
    pid_t pid = 1;

    while (pid > 0)
    { /* claim as many zombies as we can */
        pid = waitpid(0, (int *)0, WNOHANG);
    }
}

int daemon_init()
{
    pid_t pid;
    struct sigaction act;

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

    act.sa_handler = claim_children; /* use reliable signal */
    sigemptyset(&act.sa_mask);       /* not to block other signals */
    act.sa_flags = SA_NOCLDSTOP;     /* not catch stopped children */
    sigaction(SIGCHLD, (struct sigaction *)&act, (struct sigaction *)0);

    return OK;
}

int startServerProg(int argumentCount, char *argumentValue[])
{
    int socketStatus, messageSize, serverSocket, cliConnSocket, recipientLen, senderLen;
    char *accessLog = "svr_access_log.txt", *errorLog = "svr_error_log.txt";
    char programTime[MAX_TIME] = "";
    char mesgFromUser[MAX_BLOCK_SIZE], commandFromMesg[MAX_BLOCK_SIZE], argumentFromMesg[MAX_BLOCK_SIZE];
    pid_t pid;

    struct sockaddr *recipient, *sender;

    currentTime(programTime);
    svrAccessLog = fopen(accessLog, "a");
    svrErrorLog = fopen(errorLog, "a");

    if (argumentCount == 1)
    {
        chdir("/");
    }
    else if (argumentCount == 2)
    {
        if (chdir(argumentValue[1]) < 0)
        {
            perror("Path error: ");
            return PATH_ERROR;
        }
    }

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
        printf("Unable to convert into daemon\n"), exit(4);
    }

    fprintf(svrAccessLog, "%s server pid = %d\n", programTime, getpid());
    fflush(svrAccessLog);
    fprintf(svrAccessLog, "%s Initialise server FTP program...\n", programTime);
    fflush(svrAccessLog);

    socketStatus = initServerProg(&serverSocket, programTime);
    if (socketStatus != 0)
    {
        printf("Socket error. This program will be terminated.\n");
        exit(socketStatus);
    }

    fprintf(svrAccessLog, "%s Socket created successfully and ready to accept connection from clients.\n", programTime);
    fflush(svrAccessLog);

    while (1)
    {
        senderLen = sizeof(sender);
        struct sockaddr *sender_addr = (struct sockaddr *)sender;
        cliConnSocket = accept(serverSocket, sender_addr, (socklen_t *)&senderLen);
        if (cliConnSocket < 0)
        {
            if (errno == EINTR)
            {
                fprintf(svrAccessLog, "%s SIGCHLD interrupted", programTime);
                continue;
            }

            perror("Error in accepting connection from clients: ");
            printf("Program is terminating...\n");
            close(serverSocket);
            return ACCEPT_FAILED;
        }

        if ((pid = fork()) < 0)
        {
            perror("fork error");
            exit(FORK_ERROR);
        }
        else if (pid > 0)
        {
            close(cliConnSocket);
            continue;
        }

        do
        {
            currentTime(programTime);
            //socketStatus = svrRecvMessage(cliConnSocket, mesgFromUser, sizeof(mesgFromUser), &messageSize, sender, &senderLen, programTime);
            /*socketStatus = read(cliConnSocket, mesgFromUser, sizeof(mesgFromUser));
            if (socketStatus < 0)
            {
                printf("Failed to receive messages. Please check again.\n");
                fprintf(svrErrorLog, "%s Unable to receive message.", programTime);
                fflush(svrErrorLog);
                break;
            }*/
            serve_client(cliConnSocket, sender_addr);
        } while (1);
    }

    close(serverSocket);
    return OK;
}

int initServerProg(int *socketNum, char *initTime)
{
    int sock;
    struct sockaddr_in serverAddr;
    currentTime(initTime);

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Failed to create socket");
        fprintf(svrErrorLog, "%s Failed to create socket\n", initTime);
        fflush(svrErrorLog);
        return (CREATE_SOCKET_FAILED);
    }

    memset((char *)&serverAddr, 0, sizeof(serverAddr));

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(SERVER_FTP_PORT);

    if (bind(sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        perror("Unable to bind socket to the server IP");
        fprintf(svrErrorLog, "%s Unable to bind socket to the server IP\n", initTime);
        fflush(svrErrorLog);
        close(sock);
        return (BIND_FAILED);
    }

    listen(sock, serverQueue);
    *socketNum = sock;
    return (OK);
}

void serve_client(int socketNum, struct sockaddr *addr)
{
    int nr, nw;
    char buf[MAX_BLOCK_SIZE];
    char serveTime[MAX_TIME] = "";
    struct sockaddr_in *sendAddr = (struct sockaddr_in *)&addr;
    struct in_addr visitorIP = sendAddr->sin_addr;

    while (1)
    {
        if ((nr = read(socketNum, buf, sizeof(buf))) <= 0)
            exit(0);
        currentTime(serveTime);
        fprintf(svrAccessLog, "%s Message processed successfully - %s.\n", serveTime, inet_ntoa(visitorIP));
        fflush(svrAccessLog);
        /* process data */
        buf[nr] = '\0';

        /* send results to client */
        nw = write(socketNum, buf, nr);
    }
}

int svrSendMessage(int socketNum, char *bufMesg, int mesgSize, struct sockaddr *recipient, int recipientLeng, char *sendTime)
{
    recipientLeng = sizeof(recipient);
    struct sockaddr_in *recipient_addr = (struct sockaddr_in *)recipient;
    for (int i = 0; i < mesgSize; ++i)
    {
        printf("%c", bufMesg[i]);
        fprintf(svrAccessLog, "%s Message %c sent to the client %s\n", sendTime, bufMesg[i], inet_ntoa((struct in_addr)recipient_addr->sin_addr));
        fflush(svrAccessLog);
    }
    printf("\n");

    if ((sendto(socketNum, bufMesg, mesgSize, 0, recipient, recipientLeng)) < 0)
    {
        fprintf(svrErrorLog, "%s Unable to send file to client %s\n", sendTime, inet_ntoa((struct in_addr)recipient_addr->sin_addr));
        fflush(svrErrorLog);
        return (SEND_FAILED);
    }
    return (OK);
}

int svrRecvMessage(int socketNum, char *bufMesg, int bufferSize, int *mesgSize, struct sockaddr *sender, int *senderLeng, char *receiveTime)
{
    //*senderLeng = sizeof(sender);
    struct sockaddr_in *sender_addr = (struct sockaddr_in *)sender;
    *mesgSize = recvfrom(socketNum, bufMesg, bufferSize, 0, sender, senderLeng);

    if (*mesgSize < 0)
    {
        fprintf(svrErrorLog, "%s Unable to receive file from client %s\n", receiveTime, inet_ntoa((struct in_addr)sender_addr->sin_addr));
        fflush(svrErrorLog);
        return (RECEIVE_FAILED);
    }

    for (int i = 0; i < *mesgSize; ++i)
    {
        printf("%c", bufMesg[i]);
        fprintf(svrAccessLog, "%s Message %c receive from the client %s\n", receiveTime, bufMesg[i], inet_ntoa((struct in_addr)sender_addr->sin_addr));
        fflush(svrAccessLog);
    }
    printf("\n");

    return (OK);
}

void currentTime(char *timeNow)
{
    int hours, minutes, seconds, day, month, year;
    time_t now;
    time(&now);
    struct tm *local = localtime(&now);
    strftime(timeNow, MAX_TIME, "%Y:%m:%d %H:%M:%S", local);
}