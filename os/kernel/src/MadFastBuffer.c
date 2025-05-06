#include "MadOS.h"

#define MAD_ALIGNED_FBNODE_SIZE    MAD_ALIGNED_SIZE(sizeof(MadFBNode_t))
#define MAD_ALIGNED_FBBUFFER_SIZE  MAD_ALIGNED_SIZE(sizeof(MadFBuffer_t))

MadFBuffer_t* madFBufferCreate(MadSize_t n, MadSize_t size)
{
    MadSize_t    i;
    MadSize_t    step;
    MadSize_t    n_real;
    MadSize_t    s_real;
    MadVptr      res;
    MadU8        *data;
    MadFBuffer_t *fb;
    MadFBNode_t  *node;   
    if((0 == n) || (0 == size))
        return MNULL;
    s_real = MAD_ALIGNED_SIZE(size);
    step   = MAD_ALIGNED_FBNODE_SIZE + s_real;
    n_real = MAD_ALIGNED_FBBUFFER_SIZE + n * step;
    res = madMemMalloc(n_real);
    if(MNULL == res)
        return MNULL;
    fb       = (MadFBuffer_t*)res;
    data     = (MadU8*)res + MAD_ALIGNED_FBBUFFER_SIZE;
    fb->n    = n;
    fb->max  = n;
    fb->head = (MadFBNode_t*)data;
    node     = fb->head;
    for(i=1; i<n; i++) {
        data += step;
        node->next = (MadFBNode_t*)data;
        node = node->next;
    }
    node->next = MNULL;
    return fb;
}

MadVptr madFBufferGet(MadFBuffer_t *fb)
{
    MadU8 *res;
    madCSDecl(cpsr);
    madCSLock(cpsr);
    if((MNULL == fb) || (MNULL == fb->head)) {
        madCSUnlock(cpsr);
        return MNULL;
    }
    res = (MadU8*)fb->head;
    fb->head = fb->head->next;
    fb->n--;
    res += MAD_ALIGNED_FBNODE_SIZE;
    madCSUnlock(cpsr);
    return res;
}

void madFBufferPut(MadFBuffer_t *fb, MadVptr buf)
{
    MadFBNode_t *node;
    madCSDecl(cpsr);
    madCSLock(cpsr);
    if((MNULL == fb) || (MNULL == buf)) {
        madCSUnlock(cpsr);
        return;
    }
    node = (MadFBNode_t*)((MadU8*)buf - MAD_ALIGNED_FBNODE_SIZE);
    node->next = fb->head;
    fb->head = node;
    fb->n++;
    madCSUnlock(cpsr);
}
