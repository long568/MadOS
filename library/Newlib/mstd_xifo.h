#ifndef __COM_LIFO__H__
#define __COM_LIFO__H__

#include <string.h>
#include "MadOS.h"

/*
 * LIFO_U8
 */
typedef struct {
    MadU8  *buf;
    MadU8  *org;
    MadU16  index;
    MadU16  max;
    MadU16  blk_s;
    MadU16  blk_n;
    MadU16  blk_i;
} LIFO_U8;

extern  LIFO_U8 *LIFO_U8_Create(MadU16 size);
extern  LIFO_U8 *LIFO_U8_CreateBlock(MadU16 size, MadU16 num);
#define LIFO_U8_Delete(lifo)  madMemFreeNull(lifo);
#define LIFO_U8_Put(lifo, c)  do { if ((lifo)->index < (lifo)->max) (lifo)->buf[(lifo)->index++] = c; } while(0)
#define LIFO_U8_Get(lifo, c)  do { if ((lifo)->index >         0) c = (lifo)->buf[--(lifo)->index]; } while(0)
#define LIFO_U8_Clear(lifo)   do { (lifo)->index = 0; } while(0);
#define LIFO_U8_Full(lifo)    ((lifo)->index == (lifo)->max)
#define LIFO_U8_Empty(lifo)   ((lifo)->index == 0)
#define LIFO_U8_Cnt(lifo)     ((lifo)->index)
#define LIFO_U8_Max(lifo)     ((lifo)->max)
#define LIFO_U8_Buf(lifo)     ((lifo)->buf)
#define LIFO_U8_Last(lifo)    ((lifo)->buf[(lifo)->index-1])
#define LIFO_U8_CurBlock(lifo)  (lifo)->buf
#define LIFO_U8_NextBlock(lifo) do { if(++(lifo)->blk_i < (lifo)->blk_n ) { (lifo)->buf += (lifo)->blk_s; } \
                                     else { (lifo)->buf = (lifo)->org; (lifo)->blk_i = 0; } \
                                     (lifo)->index = 0; \
                                   } while(0)

/*
 * FIFO_U8
 */
typedef struct {
    MadU8  *buf;
    MadU8  *head;
    MadU8  *tail;
    MadU8  *end;
    MadU16  cnt;
    MadU16  max;
} FIFO_U8;

extern  FIFO_U8* FIFO_U8_Create        (MadU16 size);
#define          FIFO_U8_Delete(fifo)  madMemFreeNull(fifo);
extern  void     FIFO_U8_Init          (FIFO_U8 *fifo, void *buf, MadU16 size);
extern  void     FIFO_U8_Shut          (FIFO_U8 *fifo);
#define FIFO_U8_Put(fifo, c)     do { if ((fifo)->cnt < (fifo)->max) { *(fifo)->tail++ = c; (fifo)->cnt++; if((fifo)->tail == (fifo)->end) (fifo)->tail = (fifo)->buf; } } while(0)
#define FIFO_U8_Get(fifo, c)     do { if ((fifo)->cnt >         0) { c = *(fifo)->head++; (fifo)->cnt--; if((fifo)->head == (fifo)->end) (fifo)->head = (fifo)->buf; } } while(0)
#define FIFO_U8_Get2(fifo, c)    do { c = *(fifo)->head++; if((fifo)->head == (fifo)->end) (fifo)->head = (fifo)->buf; } while(0)
#define FIFO_U8_Full(fifo)       ((fifo)->cnt == (fifo)->max)
#define FIFO_U8_Empty(fifo)      ((fifo)->cnt == 0)
#define FIFO_U8_Cnt(fifo)        ((fifo)->cnt)
#define FIFO_U8_Max(fifo)        ((fifo)->max)
#define FIFO_U8_Buf(fifo)        ((fifo)->buf)
#define FIFO_U8_Clear(fifo)      do { (fifo)->cnt = 0; (fifo)->head = (fifo)->tail; } while(0);
#define FIFO_U8_Reset(fifo)      do { (fifo)->cnt = 0; (fifo)->head = (fifo)->tail = (fifo)->buf; } while(0);

/* Called in ISR-Mode */
#define FIFO_U8_DMA_Put(fifo, n) do { \
    if((fifo)->cnt < (fifo)->max) {                                             \
        MadU8 *tmp;                                                             \
        MadU32 ofs = (fifo)->max - (fifo)->cnt;                                 \
        if(ofs > n) { tmp = (fifo)->tail + n;   (fifo)->cnt += n;            }  \
        else        { tmp = (fifo)->tail + ofs; (fifo)->cnt += ofs; n = ofs; }  \
        if(tmp < (fifo)->end) (fifo)->tail = tmp;                               \
        else                  (fifo)->tail = (fifo)->buf + (tmp - (fifo)->end); \
    } else {                                                                    \
        n = 0;                                                                  \
    }                                                                           \
} while(0)

/* Called in User-Mode */
#define FIFO_U8_DMA_Get(fifo, dat, n) do { \
    madCSDecl(cpsr);                                  \
    madCSLock(cpsr);                                  \
    if((fifo)->cnt > 0) {                             \
        if(n > (fifo)->cnt) { n = (fifo)->cnt; }      \
        (fifo)->cnt -= n;                             \
        madCSUnlock(cpsr);                            \
        if((fifo)->head + n < (fifo)->end)  {         \
            memcpy(dat, (fifo)->head, n);             \
            (fifo)->head += n;                        \
        } else {                                      \
            MadU32 ofs = (fifo)->end - (fifo)->head;  \
            memcpy(dat,     (fifo)->head, ofs);       \
            memcpy(dat+ofs, (fifo)->buf,  n - ofs);   \
            (fifo)->head = (fifo)->buf + n - ofs;     \
        }                                             \
    } else {                                          \
        madCSUnlock(cpsr);                            \
        n = 0;                                        \
    }                                                 \
} while(0)

#endif
