#include <sys/types.h>
#include <netinet/in.h> /* struct sockaddr_in, htons(), htonl(), */
#include <stdlib.h>
#include <unistd.h>

#include "protocol.h"

int readContent(int fd, char *buf, int bufsize)
{
    short data_size; /* sizeof (short) must be 2 */
    int n, nr, len;

    /* check buffer size len */
    if (bufsize < MAX_BLOCK_SIZE) //bufsize being read is bigger than max size
        return (EXCEEDED_ALLOWED_SIZE);

    /* get the size of data sent*/
    if (read(fd, (char *)&data_size, 1) != 1)
        return (-1);
    if (read(fd, (char *)(&data_size) + 1, 1) != 1)
        return (-1);
    len = (int)ntohs(data_size); //convert to host byte order

    /* read len number of bytes to buf */
    for (n = 0; n < len; n += nr)
    {
        if ((nr = read(fd, buf + n, len - n)) <= 0)
            return (nr); /* error in reading */
    }
    return (len); //return the amount of characters/bytes read if read is successful.
}

int writeContent(int fd, char *buf, int nbytes)
{
    short data_size = nbytes; /* short must be two bytes long */
    int n, nw;

    if (nbytes > MAX_BLOCK_SIZE) //nbytes being writen is larger than max block size
        return (EXCEEDED_ALLOWED_SIZE);

    /* send the data size */
    data_size = htons(data_size); //convert to network byte order
    if (write(fd, (char *)&data_size, 1) != 1)
        return (-1);
    if (write(fd, (char *)(&data_size) + 1, 1) != 1)
        return (-1);

    /* send nbytes */
    for (n = 0; n < nbytes; n += nw)
    {
        if ((nw = write(fd, buf + n, nbytes - n)) <= 0)
            return (nw); /* write error */
    }
    return (n); //return the amount of characters/bytes writen if write is successful
}