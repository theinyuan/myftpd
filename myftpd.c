/*
 *  File: myftpd.c (server program)
 *  Authors: Russell and TY
 *  Date Start: 12 Nov 2021
 *  Purpose: Project 2 - A Simple File Transfer Protocol
 *  Description: a simple network protocol that can be used to download files from a remote site and to upload files to a remote site, and a client and a server programs that communicate using that protocol.
 */

#include <stdio.h>
#include <sys/types.h>
#include "protocol.h"

// #define SERV_INET_NO
// #define SERV_TCP_PORT 

int main()
{
    // creating daemon process
    pid_t pid;

    if((pid = fork()) < 0)
    {
        perror("fork() error");
        exit(1);
    }
    else if(pid > 0) // parent process
    {
        exit(0);
    }
    
    // child process continues
    setsid(); // establish itself as session leader
    chdir("/"); // changes working directory to root
    umask(0); // clear file mode creation mask

    return 0;
}