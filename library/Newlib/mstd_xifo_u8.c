#include "mstd_xifo.h"

LIFO_U8 *LIFO_U8_Create(MadU16 size)
{
    LIFO_U8 *lifo;
    MadU16   head_size;
    head_size = MAD_ALIGNED_SIZE(sizeof(LIFO_U8));
    lifo = (LIFO_U8*)madMemMalloc(head_size + size);
    if(lifo == 0) return 0;
    lifo->buf   = (MadU8*)lifo + head_size;
    lifo->org   = lifo->buf;
    lifo->index = 0;
    lifo->max   = size;
    lifo->blk_s = size;
    lifo->blk_n = 1;
    lifo->blk_i = 0;
    return lifo;
}

LIFO_U8 *LIFO_U8_CreateBlock(MadU16 size, MadU16 num)
{
    LIFO_U8 *lifo = LIFO_U8_Create(size * num);
    lifo->max   = size;
    lifo->blk_s = size;
    lifo->blk_n = num;
    lifo->blk_i = 0;
    return lifo;
}

FIFO_U8 *FIFO_U8_Create(MadU16 size)
{
    FIFO_U8 *fifo;
    MadU16  head_size;
    head_size = MAD_ALIGNED_SIZE(sizeof(FIFO_U8));
    fifo = (FIFO_U8*)madMemMalloc(head_size + size);
    if(fifo == 0) return 0;
    fifo->buf  = (MadU8*)fifo + head_size;
    fifo->head = fifo->buf;
    fifo->tail = fifo->buf;
    fifo->end  = fifo->buf + size;
    fifo->cnt  = 0;
    fifo->max  = size;
    return fifo;
}

void FIFO_U8_Init(FIFO_U8 *fifo, void *buf, MadU16 size)
{
    MAD_CS_OPT(
        fifo->buf  = (MadU8*)buf;
        fifo->head = fifo->buf;
        fifo->tail = fifo->buf;
        fifo->end  = fifo->buf + size;
        fifo->cnt  = 0;
        fifo->max  = size;
    );
}

void FIFO_U8_Shut(FIFO_U8 *fifo)
{
    MAD_CS_OPT(
        fifo->buf  = 0;
        fifo->head = 0;
        fifo->tail = 0;
        fifo->end  = 0;
        fifo->cnt  = 0;
        fifo->max  = 0;
    );
}
