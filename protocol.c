#include <time.h>
#include "protocol.h"

void currentTime(char *timeNow)
{
    int hours, minutes, seconds, day, month, year;
    time_t now;
    time(&now);
    struct tm *local = localtime(&now);
    strftime(timeNow, MAX_TIME, "%Y:%m:%d %H:%M:%S", local);
}