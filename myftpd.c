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
void currentTime(char *timeNow);

FILE *svrAccessLog;
FILE *outputFile;
char *space = " ";

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
    char *accessLog = "svr_access_log.txt";
    char programTime[MAX_TIME] = "";
    char mesgFromUser[MAX_BLOCK_SIZE], commandFromMesg[MAX_BLOCK_SIZE], argumentFromMesg[MAX_BLOCK_SIZE], replyMessage[MAX_BLOCK_SIZE];
    pid_t pid;

    struct sockaddr *recipient, *sender;

    currentTime(programTime);
    svrAccessLog = fopen(accessLog, "a");

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
        currentTime(programTime);
        senderLen = sizeof(sender);
        cliConnSocket = accept(serverSocket, (struct sockaddr *)sender, &senderLen);
        if (cliConnSocket < 0)
        {
            if (errno == EINTR)
            {
                fprintf(svrAccessLog, "%s SIGCHLD interrupted", programTime);
                continue;
            }

            perror("Error in accepting connection from clients: ");
            close(serverSocket);
            return ACCEPT_FAILED;
        }

        fprintf(svrAccessLog, "%s Client connected.\n", programTime);
        fflush(svrAccessLog);

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
            socketStatus = read(cliConnSocket, mesgFromUser, sizeof(mesgFromUser));
            mesgFromUser[socketStatus] = '\0';
            if (strchr(mesgFromUser, ' ') == NULL)
            {
                strcpy(commandFromMesg, mesgFromUser);
            }
            else
            {
                strcpy(commandFromMesg, strtok(mesgFromUser, space));
                strcpy(argumentFromMesg, strtok(NULL, space));
            }
            //serve_client(cliConnSocket, sender_addr);
            if (strcmp(commandFromMesg, "pwd") == 0)
            {
                currentTime(programTime);
                memset(replyMessage, '\0', sizeof(replyMessage));
                if (system("pwd > /tmp/pwd.txt") < 0)
                {
                    perror("System error: ");
                    exit(1);
                }
                outputFile = fopen("/tmp/pwd.txt", "r");
                socketStatus = fread(replyMessage, sizeof(replyMessage), sizeof(char), outputFile);
                strtok(replyMessage, "\n");
                write(cliConnSocket, replyMessage, sizeof(replyMessage));
                fprintf(svrAccessLog, "%s Message %s received successfully.\n", programTime, commandFromMesg);
                fprintf(svrAccessLog, "%s Message %s sent successfully.\n", programTime, replyMessage);
                fflush(svrAccessLog);
                fclose(outputFile);
                system("rm /tmp/pwd.txt");
                //memset(commandFromMesg, '\0', sizeof(commandFromMesg));
                close(socketStatus);
            }
            else if (strcmp(commandFromMesg, "dir") == 0)
            {
                currentTime(programTime);
                memset(replyMessage, '\0', sizeof(replyMessage));
                if (system("dir > /tmp/dir.txt") < 0)
                {
                    perror("System error: ");
                    exit(1);
                }
                outputFile = fopen("/tmp/dir.txt", "r");
                socketStatus = fread(replyMessage, sizeof(replyMessage), sizeof(char), outputFile);
                write(cliConnSocket, replyMessage, sizeof(replyMessage));
                fprintf(svrAccessLog, "%s Message %s received successfully.\n", programTime, commandFromMesg);
                fprintf(svrAccessLog, "%s Message %s sent successfully.\n", programTime, replyMessage);
                fflush(svrAccessLog);
                fclose(outputFile);
                system("rm /tmp/dir.txt");
                memset(commandFromMesg, '\0', sizeof(commandFromMesg));
            }
            else if (strcmp(commandFromMesg, "cd") == 0)
            {
                currentTime(programTime);
                socketStatus = chdir(argumentFromMesg);
                if (socketStatus < 0)
                {
                    memset(replyMessage, '\0', sizeof(replyMessage));
                    strcpy(replyMessage, "Invalid directory");
                    write(cliConnSocket, replyMessage, sizeof(replyMessage));
                    fprintf(svrAccessLog, "%s %s directory does not exist.\n", programTime, argumentFromMesg);
                    fflush(svrAccessLog);
                }
                //memset(commandFromMesg, '\0', sizeof(commandFromMesg));
                //memset(argumentFromMesg, '\0', sizeof(argumentFromMesg));
            }
            else
            {
                currentTime(programTime);
                int writeStatus = write(cliConnSocket, commandFromMesg, socketStatus);
                fprintf(svrAccessLog, "%s Message %s received successfully.\n", programTime, mesgFromUser);
                fprintf(svrAccessLog, "%s Message %s sent successfully.\n", programTime, commandFromMesg);
                fflush(svrAccessLog);
            }
        } while (strcmp(commandFromMesg, "quit") != 0);
        fprintf(svrAccessLog, "%s Client disconnected.\n", programTime);
        fflush(svrAccessLog);
    }
    fclose(svrAccessLog);
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
        return (CREATE_SOCKET_FAILED);
    }

    bzero((char *)&serverAddr, sizeof(serverAddr));

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(SERVER_FTP_PORT);

    if (bind(sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        perror("Unable to bind socket to the server IP");
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

void currentTime(char *timeNow)
{
    int hours, minutes, seconds, day, month, year;
    time_t now;
    time(&now);
    struct tm *local = localtime(&now);
    strftime(timeNow, MAX_TIME, "%Y:%m:%d %H:%M:%S", local);
}
