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

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>

#include "protocol.h"

// functions declarations
int startClientProg(int argumentCount, char *argumentValue[]);
int initClientProg(char *serverName, int *socketNum);
void processCommands(int socketNum, char *input, char cmd[], char arg[]);
void pwdCommand(int socketNum, char *input, int size);
void dirCommand(int socketNum, char *input, int size);
void lpwdCommand();
void ldirCommand();
void lcdCommand(char arg[]);
void cdCommand(int socketNum, char cmd[], char arg[]);
void putCommand(int serSocket, char cmd[], char param[]);
void getCommand(int serSocket, char cmd[], char param[]);
// end of function declaration

// main begins here
int main(int argc, char *argv[])
{
    startClientProg(argc, argv);
    return 0;
}
// end of main

// functions implementation
int startClientProg(int argumentCount, char *argumentValue[])
{
    int socketStatus, sd;
    char userInput[MAX_BLOCK_SIZE], command[MAX_BLOCK_SIZE], argument[MAX_BLOCK_SIZE], host[MAX_BLOCK_SIZE];

    if (argumentCount == 1)
    {
        gethostname(host, sizeof(host));
    }
    else if (argumentCount == 2)
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
    if (socketStatus != 0)
    {
        printf("Failed to connect to the server. Terminating Program.\n");
        exit(socketStatus);
    }

    while(1)
    {
        printf("> ");
        processCommands(sd, userInput, command, argument);
    }

    return OK;
}

int initClientProg(char *serverName, int *socketNum)
{
    int sock;
    struct sockaddr_in serverAddr;
    struct hostent *hp;

    if ((hp = gethostbyname(serverName)) == NULL)
    {
        printf("host %s not found.\n", serverName);
        return (INVALID_HOST_NAME);
    }

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Failed to create socket");
        return (CREATE_SOCKET_FAILED);
    }

    bzero((char *)&serverAddr, sizeof(serverAddr));

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_FTP_PORT);
    serverAddr.sin_addr.s_addr = *(u_long *)hp->h_addr;

    if (connect(sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        perror("Failed to connect to server");
        return (CONNECT_FAILED);
    }

    *socketNum = sock;

    return (OK);
}

void processCommands(int socketNum, char *input, char cmd[], char arg[])
{
    int nr, nw;
    fgets(input, MAX_BLOCK_SIZE, stdin);
    nr = strlen(input);

    if(input[nr - 1] == '\n')
    {
        input[nr - 1] = '\0'; // stripping newline
        nr--;
    }

    if(nr > 0)
    {
        if(strcmp(input, "quit") == 0)
        {
            close(socketNum);
            printf("You are now exiting the client. Goodbye!\n");
            exit(OK);
        }
        else if(strcmp(input, "pwd") == 0)
        {
            pwdCommand(socketNum, input, nr);
        }
        else if(strcmp(input, "lpwd") == 0)
        {
            lpwdCommand();
            return;
        }
        else if(strcmp(input, "dir") == 0)
        {
            dirCommand(socketNum, input, nr);
        }
        else if(strcmp(input, "ldir") == 0)
        {
            ldirCommand();
            return;
        }
        else
        {
            printf("Invalid command");
            exit(1);
        }
        
        if(strstr(input, " ") != NULL)
        {
            strcpy(cmd, strtok(input, " "));
            // printf("%s\n", cmd); // printf to test and verify tokenizer to cmd[]
            strcpy(arg, strtok(NULL, "\0"));
            // printf("%s\n", arg); // printf to test and verify tokenizer to arg[]

            if(strcmp(cmd, "cd") == 0)
            {
                cdCommand(socketNum, cmd, arg);
                bzero(cmd, strlen(cmd));
            }
            else if(strcmp(input, "lcd") == 0)
            {
                // lcd function here
                lcdCommand(arg);
                return;
            }
            else if(strcmp(cmd, "put") == 0)
            {
                putCommand(socketNum, cmd, arg);
            }
            else if(strcmp(cmd, "get") == 0)
            {
                getCommand(socketNum, cmd, arg);
            }
            else
            {
                printf("Invalid command");
                exit(1);
            }
        }
        input[nr] = '\0';
        printf("\n");
    }
}

void pwdCommand(int socketNum, char *input, int size)
{
    writeContent(socketNum, input, size);
    readContent(socketNum, input, MAX_BLOCK_SIZE);
    printf("%s", input);
}

void lpwdCommand()
{
    char cwd[MAX_BLOCK_SIZE];
    
    if(getcwd(cwd, sizeof(cwd)) == NULL)
    {
        perror("getcwd() error");
    }
    else
    {
        printf("%s\n", cwd);
    }
}

void dirCommand(int socketNum, char *input, int size)
{
    writeContent(socketNum, input, size);
    readContent(socketNum, input, MAX_BLOCK_SIZE);
    printf("%s", input);
}

void ldirCommand()
{
    char cwd[MAX_BLOCK_SIZE];
    char fileNameInDir[MAX_BLOCK_SIZE];
    char fileList[MAX_BLOCK_SIZE];
    getcwd(cwd, sizeof(cwd));

    DIR *d;
    struct dirent *currtDir;
    d = opendir(cwd);

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
    printf("%s\n", fileList);
    closedir(d);
}

void lcdCommand(char arg[])
{
    char currentDir[MAX_BLOCK_SIZE];
    int count = 0;

    if (strcmp(arg, "#") != 0)
    {
        count = 1;
    }

    if(count > 0)
    {
        if(strcmp(arg, ".") == 0)
        {
            getcwd(currentDir, sizeof(currentDir));
            //printf("%s\n", currentDir);
        }
        else if(strcmp(arg, "~") == 0)
        {
            chdir(getenv("HOME"));
            getcwd(currentDir, sizeof(currentDir));
            //printf("%s\n", currentDir);
        }
        else
        {
            if(chdir(arg) < 0)
            {
                strcat(currentDir, "Directory not found!");
                count = 2;
            }
            else
            {
                getcwd(currentDir, sizeof(currentDir));
            } //
        }
    }
    else
    {
        chdir(getenv("HOME"));
        getcwd(currentDir, sizeof(currentDir));
    }

    if(count == 2)
    {
        printf("%s Directory not found\n", arg);
    }
    else
    {
        printf("%s\n", currentDir);
    }
}

void cdCommand(int socketNum, char cmd[], char arg[])
{
    writeContent(socketNum, cmd, strlen(cmd));
    writeContent(socketNum, arg, strlen(arg));
}

void putCommand(int serSocket, char cmd[], char param[])
{
    char uploadFileName[1000] = {0};
    char fileStatus[MAX_BLOCK_SIZE] = {0};
    char currentPath[MAX_BLOCK_SIZE] = {0};
    int similarName = 0;

    getcwd(currentPath, sizeof(currentPath));

    DIR *d;
    struct dirent *currtDir;
    d = opendir(currentPath);

    if (d)
    {
        while ((currtDir = readdir(d)) != NULL)
        {
            if (strcmp(currtDir->d_name, ".") != 0 && strcmp(currtDir->d_name, "..") != 0)
            {
                if (strcmp(currtDir->d_name, param) == 0)
                {
                    similarName = 1;
                }
            }
        }
    }
    closedir(d);

    if (similarName > 0)
    {
        writeContent(serSocket, cmd, strlen(cmd));
        writeContent(serSocket, param, strlen(param));
        readContent(serSocket, fileStatus, sizeof(fileStatus));

        if (strcmp(fileStatus, FILE_NO_CONFLICT) == 0)
        {
            FILE *uploadFile;
            uploadFile = fopen(param, "r");
            if (uploadFile == NULL)
            {
                printf("File error. Please try again.\n");
            }

            while ((fread(uploadFileName, sizeof(char), sizeof(uploadFileName), uploadFile)) > 0)
            {
                writeContent(serSocket, uploadFileName, strlen(uploadFileName));
                bzero(uploadFileName, sizeof(uploadFileName));
            }
            printf("File %s uploaded successfully.\n", param);
            writeContent(serSocket, EOF_MESSAGE, sizeof(EOF_MESSAGE));
            fclose(uploadFile);
        }
        else if (strcmp(fileStatus, FILE_ALREADY_EXIST) == 0)
        {
            printf("File %s already exist in the server, please rename or choose another file.\n", param);
        }
        else
        {
            printf("Unexpected error occured, please try again later.\n");
        }
    }
    else
    {
        printf("No such file as %s is found in your device, please choose another file.\n", param);
    }
}

void getCommand(int serSocket, char cmd[], char param[])
{
    char currentPath[MAX_BLOCK_SIZE] = {0};
    int similarName = 0;

    DIR *d;
    struct dirent *currtDir;
    d = opendir(currentPath);

    if (d)
    {
        while ((currtDir = readdir(d)) != NULL)
        {
            if (strcmp(currtDir->d_name, ".") != 0 && (strcmp(currtDir->d_name, "..") != 0))
            {
                if (strcmp(currtDir->d_name, param) == 0)
                {
                    similarName = 1;
                }
            }
        }
    }

    closedir(d);

    if (similarName == 0)
    {
        char fileStatus[MAX_BLOCK_SIZE] = {0};
        char downloadFileName[MAX_BLOCK_SIZE] = {0};
        writeContent(serSocket, cmd, strlen(cmd));
        writeContent(serSocket, param, strlen(param));
        readContent(serSocket, fileStatus, sizeof(fileStatus));
        if (strcmp(fileStatus, FILE_NO_CONFLICT) == 0)
        {
            FILE *downloadFile;
            downloadFile = fopen(param, "w");
            readContent(serSocket, downloadFileName, sizeof(downloadFileName));
            if (strcmp(downloadFileName, FILE_ERROR) == 0)
            {
                printf("Error downloading the file, please try again later.\n");
            }
            else
            {
                while (strcmp(downloadFileName, EOF_MESSAGE) != 0)
                {
                    fwrite(downloadFileName, sizeof(char), sizeof(downloadFileName), downloadFile);
                    bzero(downloadFileName, sizeof(downloadFileName));
                    readContent(serSocket, downloadFileName, sizeof(downloadFileName));
                }
                printf("File %s downloaded successfully.\n", downloadFileName);
                bzero(downloadFileName, sizeof(downloadFileName));
            }
            fclose(downloadFile);
        }
        else
        {
            printf("File %s not found on the server, please try with another file.\n", param);
        }
    }
    else
    {
        printf("The same file is existed in the device, please rename it and try again.\n");
    }
}