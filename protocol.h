#ifndef PROTOCOL_H
#define PROTOCOL_H

/*
 *  Author: Chang Thein Yuan
 *  Date: 13 November 2021
 *  Purpose: Reference for the client and server program to communicate
 *  Protocol used: TCP
 */

#include <stdio.h>

#define OK 0
#define INVALID_HOST_NAME -1
#define CREATE_SOCKET_FAILED -2
#define BIND_FAILED -3
#define CONNECT_FAILED -4
#define SEND_FAILED -5
#define RECEIVE_FAILED -6
#define ACCEPT_FAILED -7

char userCmd[1024];  /* user typed ftp command line received from client */
char cmd[1024];      /* ftp command (without argument) extracted from userCmd */
char argument[1024]; /* argument (without ftp command) extracted from userCmd */
char replyMsg[4096]; /* buffer to send reply message to client */
char *space = " ";
char buffer[4096];
FILE *myfile;

#endif