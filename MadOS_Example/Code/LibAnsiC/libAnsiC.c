#include "MadOS.h"
#include "stm32_ttyUSART.h"

#include <stdio.h> /* ---------------------------------- */

int ungetc(int c, FILE * stream)
{
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
