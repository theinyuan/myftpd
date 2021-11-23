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

#include <dirent.h>
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
int initServerProg(int *socketNum);
void serve_client(int socketNum);
void currentTime(char *timeNow);
void pwdCommand(int cliSocket);
void dirCommand(int cliSocket);
void cdCommand(int cliSocket);
void getCommand(int cliSocket);
void putCommand(int cliSocket);

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
    char childTime[MAX_TIME] = "";
    currentTime(childTime);

    while (pid > 0)
    { /* claim as many zombies as we can */
        pid = waitpid(0, (int *)0, WNOHANG);
    }

    fprintf(svrAccessLog, "%s End of process\n", childTime);
    fflush(svrAccessLog);
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
    umask(0);

    act.sa_handler = claim_children; /* use reliable signal */
    sigemptyset(&act.sa_mask);       /* not to block other signals */
    act.sa_flags = SA_NOCLDSTOP;     /* not catch stopped children */
    sigaction(SIGCHLD, (struct sigaction *)&act, (struct sigaction *)0);

    return OK;
}

int startServerProg(int argumentCount, char *argumentValue[])
{
    int socketStatus, serverSocket, cliConnSocket;
    char *accessLog = "svr_access_log.txt";
    char programTime[MAX_TIME] = "";
    //char mesgFromUser[MAX_BLOCK_SIZE], commandFromMesg[MAX_BLOCK_SIZE], argumentFromMesg[MAX_BLOCK_SIZE], replyMessage[MAX_BLOCK_SIZE];
    socklen_t senderLen;
    pid_t pid;

    struct sockaddr_in sender;

    currentTime(programTime);
    svrAccessLog = fopen(accessLog, "a");

    if (argumentCount == 1)
    {
        char currentPath[PATH_MAX];
        getcwd(currentPath, sizeof(currentPath));
        chdir(currentPath);
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

    socketStatus = initServerProg(&serverSocket);
    if (socketStatus != 0)
    {
        printf("Socket error. This program will be terminated.\n");
        exit(socketStatus);
    }
    fprintf(svrAccessLog, "%s server pid = %d\n", programTime, getpid());
    fprintf(svrAccessLog, "%s Initialise server FTP program...\n", programTime);
    fprintf(svrAccessLog, "%s Socket created successfully and ready to accept connection from clients.\n", programTime);
    fflush(svrAccessLog);

    while (1)
    {
        senderLen = sizeof(sender);
        cliConnSocket = accept(serverSocket, (struct sockaddr *)&sender, &senderLen);
        if (cliConnSocket < 0)
        {
            if (errno == EINTR)
            {
                currentTime(programTime);
                fprintf(svrAccessLog, "%s SIGCHLD interrupted\n", programTime);
                continue;
            }

            perror("Error in accepting connection from clients: ");
            return ACCEPT_FAILED;
        }
        currentTime(programTime);
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
        close(serverSocket);
        serve_client(cliConnSocket);
        fclose(svrAccessLog);
        return OK;
    }
}

int initServerProg(int *socketNum)
{
    int sock;
    struct sockaddr_in serverAddr;

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

void serve_client(int socketNum)
{
    int nr, nw;
    char buf[MAX_BLOCK_SIZE];

    while (1)
    {
        if ((nr = readContent(socketNum, buf, sizeof(buf))) <= 0)
            return;

        /* process data */
        buf[nr] = '\0';
        if (strcmp(buf, "pwd") == 0)
        {
            bzero(buf, sizeof(buf));
            pwdCommand(socketNum);
        }
        else if (strcmp(buf, "dir") == 0)
        {
            bzero(buf, sizeof(buf));
            dirCommand(socketNum);
        }
        else if (strcmp(buf, "cd") == 0)
        {
            bzero(buf, sizeof(buf));
            cdCommand(socketNum);
        }
        else if (strcmp(buf, "get") == 0)
        {
            bzero(buf, sizeof(buf));
            getCommand(socketNum);
        }
        else if (strcmp(buf, "put") == 0)
        {
            bzero(buf, sizeof(buf));
            putCommand(socketNum);
        }
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

void pwdCommand(int cliSocket)
{
    char pwdTime[MAX_TIME] = "";
    currentTime(pwdTime);
    char currentPathName[MAX_BLOCK_SIZE] = {0};
    getcwd(currentPathName, sizeof(currentPathName));
    writeContent(cliSocket, currentPathName, strlen(currentPathName));

    fprintf(svrAccessLog, "%s pwd command received from client.\n", pwdTime);
    fprintf(svrAccessLog, "%s %s sent to the client.\n", pwdTime, currentPathName);
    fflush(svrAccessLog);
}

void dirCommand(int cliSocket)
{
    char dirTime[MAX_TIME] = "";
    currentTime(dirTime);
    char currentDirectory[MAX_BLOCK_SIZE] = {0};
    char fileNameInDir[MAX_BLOCK_SIZE] = {0};
    char fileList[MAX_BLOCK_SIZE] = {0};
    getcwd(currentDirectory, sizeof(currentDirectory));

    DIR *d;
    struct dirent *currtDir;
    d = opendir(currentDirectory);

    if (d)
    {
        while ((currtDir = readdir(d)) != NULL)
        {
            if (strcmp(currtDir->d_name, ".") != 0 && strcmp(currtDir->d_name, "..") != 0)
            {
                strcpy(fileNameInDir, currtDir->d_name);
                strcat(fileList, fileNameInDir);
                strcat(fileList, "\n");
            }
        }
    }
    writeContent(cliSocket, fileList, strlen(fileList));
    closedir(d);

    fprintf(svrAccessLog, "%s dir command received from client.\n", dirTime);
    fprintf(svrAccessLog, "%s %ssent to the client.\n", dirTime, fileList);
    fflush(svrAccessLog);
}

void cdCommand(int cliSocket)
{
    char cdTime[MAX_TIME] = "";
    currentTime(cdTime);
    char argument[MAX_BLOCK_SIZE] = {0};
    char replyClient[MAX_BLOCK_SIZE] = {0};
    int count = 0;

    readContent(cliSocket, argument, sizeof(argument));
    if (strcmp(argument, "#") != 0)
    {
        count = 1;
    }

    if (count > 0)
    {
        if (strcmp(argument, ".") == 0)
        {
            getcwd(replyClient, sizeof(replyClient));
        }
        else if (strcmp(argument, "~") == 0)
        {
            chdir(getenv("HOME"));
            getcwd(replyClient, sizeof(replyClient));
        }
        else
        {
            if (chdir(argument) < 0)
            {
                strcat(replyClient, "Directory not found!");
                count = 2;
            }
            else
            {
                getcwd(replyClient, sizeof(replyClient));
            } //end if
        }     // end if
    }
    else
    {
        chdir(getenv("HOME"));
        getcwd(replyClient, sizeof(replyClient));
    } //end if

    writeContent(cliSocket, replyClient, strlen(replyClient));
    if (count == 2)
    {
        fprintf(svrAccessLog, "%s Directory %s not found.\n", cdTime, argument);
        fflush(svrAccessLog);
    }
    else
    {
        fprintf(svrAccessLog, "%s %s is the current directory.\n", cdTime, replyClient);
        fflush(svrAccessLog);
    }
}

void getCommand(int cliSocket)
{
    char getTime[MAX_TIME]="";
    currentTime(getTime);
    char uploadedFileName[MAX_BLOCK_SIZE]={0};
    char currentPath[MAX_BLOCK_SIZE]={0};
    int fileFound=0;

    readContent(cliSocket,uploadedFileName,sizeof(uploadedFileName));
    getcwd(currentPath,sizeof(currentPath));

    DIR *d;
    struct dirent *currtDir;
    d = opendir(currentPath);

    if (d)
    {
        while ((currtDir = readdir(d)) != NULL)
        {
            if (strcmp(currtDir->d_name, ".") != 0 && strcmp(currtDir->d_name, "..") != 0)
            {
                if(strcmp(currtDir->d_name,uploadedFileName)==0)
                {
                    fileFound++;
                }//end if
            }//end if
        }//end while
    }//end if

    closedir(d);
    bzero(currentPath,sizeof(currentPath));

    if(fileFound>0)
    {
        writeContent(cliSocket,FILE_FOUND,strlen(FILE_FOUND));
        char fileInServer[1000]={0};
        outputFile=fopen(uploadedFileName,"r");
        bzero(uploadedFileName,sizeof(uploadedFileName));
        if(outputFile == NULL)
        {
            writeContent(cliSocket,FILE_ERROR,strlen(FILE_ERROR));
        } else
        {
            while((fread(fileInServer,sizeof(char),sizeof(outputFile),outputFile))>0)
            {
                writeContent(cliSocket,fileInServer,strlen(fileInServer));
                bzero(fileInServer,sizeof(fileInServer));
            }
            writeContent(cliSocket,EOF_MESSAGE,sizeof(EOF_MESSAGE));
        }
        fclose(outputFile);
        fprintf(svrAccessLog,"%s File %s sent to client successfully.\n",getTime,uploadedFileName);
        fflush(svrAccessLog);
    } else
    {
        writeContent(cliSocket,FILE_NOT_FOUND,strlen(FILE_NOT_FOUND));
        fprintf(svrAccessLog,"%s File %s not found.\n",getTime,uploadedFileName);
        fflush(svrAccessLog);
    }
}

void putCommand(int cliSocket)
{
}