#include "MadOS.h"
#include "stm32_ttyUSART.h"

#include <stdio.h> /* ---------------------------------- */

int ungetc(int c, FILE * stream)
{
    (void) stream;
    return ttyUsart_UngetChar(c);
}

int fgetc(FILE * stream)
{
    (void) stream;
    return ttyUsart_GetChar();
}

int fputc(int c, FILE * stream)
{
    (void) stream;
    return ttyUsart_PutChar(c);
}

#include <time.h> /* ----------------------------------- */

#define MAD_STATIC_YEAR   1987
#define MAD_STATIC_MON    5
#define MAD_STATIC_DAY    11
#define MAD_STATIC_TIME() ((MadU32)(MAD_STATIC_YEAR - 1980) << 25 | (MadU32)MAD_STATIC_MON << 21 | (MadU32)MAD_STATIC_DAY << 16)

clock_t clock(void)
{
    return (clock_t)-1;
}

time_t time(time_t *timer)
{
    time_t res = MAD_STATIC_TIME();
    if(timer)
        *timer = res;
    return res;
}

#include <stdlib.h> /* --------------------------------- */

int system(const char *string)
{
    return 0;
}

void exit(int status)
{
    madThreadDeleteAndClear(MAD_THREAD_SELF);
    while(1);
}

char* getenv(const char * name)
{
    return 0;
}
