#ifndef PROTOCOL_H
#define PROTOCOL_H

/*
 *  Author: Russell Wong and Chang Thein Yuan
 *  Date: 13 November 2021
 *  Purpose: Reference for the client and server program to communicate
 *  Protocol used: TCP
 */

#include <stdio.h>
#include <time.h>

#define FORK_ERROR 1
#define OK 0
#define INVALID_HOST_NAME -1
#define CREATE_SOCKET_FAILED -2
#define BIND_FAILED -3
#define CONNECT_FAILED -4
#define SEND_FAILED -5
#define RECEIVE_FAILED -6
#define ACCEPT_FAILED -7

#define SERVER_FTP_PORT 8000
#define serverQueue 100

#define MAX_TIME 256

/*
char userCmd[1024];  //user typed ftp command line received from client
char cmd[1024];      //ftp command (without argument) extracted from userCmd
char argument[1024]; //argument (without ftp command) extracted from userCmd
char replyMsg[4096]; //buffer to send reply message to client
char *space = " ";
char buffer[4096];
*/

FILE *svrAccessLog;
FILE *svrErrorLog;
FILE *cliAccessLog;
FILE *cliErrorLog;
FILE *myfile;

void currentTime(char *timeNow);

#endif