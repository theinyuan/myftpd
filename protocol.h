#ifndef PROTOCOL_H
#define PROTOCOL_H

/*
 *  Author: Russell Wong and Chang Thein Yuan
 *  Date: 13 November 2021
 *  Purpose: Reference for the client and server program to communicate
 *  Protocol used: TCP
 */

#define EXCEEDED_ALLOWED_SIZE 2

#define FILE_ALREADY_EXIST 3 //for put command
#define FILE_NO_CONFLICT 0 //for put command

#define FILE_NOT_FOUND 1 //for get command
#define FILE_FOUND 0 //for get command

#define OK 0

#define INVALID_HOST_NAME -1
#define CREATE_SOCKET_FAILED -2
#define BIND_FAILED -3
#define CONNECT_FAILED -4
#define SEND_FAILED -5
#define RECEIVE_FAILED -6
#define ACCEPT_FAILED -7
#define FORK_ERROR -8
#define PATH_ERROR -9

#define SERVER_FTP_PORT 40001
#define serverQueue 100

#define MAX_TIME 256
#define MAX_BLOCK_SIZE (1024*5)

int readContent(int fd, char *buf, int bufsize);
int writeContent(int fd, char *buf, int nbytes);

#endif