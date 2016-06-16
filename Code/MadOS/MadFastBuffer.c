#include "MadOS.h"

MadFBuffer_t* madFBufferCreate(MadSize_t n, MadSize_t size)
{
    MadSize_t    i;
    MadSize_t    step;
    MadSize_t    n_real;
    MadVptr      res;
    MadU8        *data;
    MadFBuffer_t *fb;
    MadFBNode_t  *node;   
    if((0 == n) || (0 == size))
        return MNULL;
    step = sizeof(MadFBNode_t) + size;
    n_real = sizeof(MadFBuffer_t) + n * step;
    res = madMemMalloc(n_real);
    if(MNULL == res)
        return MNULL;   
    fb = (MadFBuffer_t*)res;
    data = (MadU8*)res + sizeof(MadFBuffer_t);
    fb->n = n;
    fb->head = (MadFBNode_t*)data;
    node = fb->head;
    for(i=1; i<n; i++) {
        data += step;
        node->next = (MadFBNode_t*)data;
        node = node->next;   
    }
    node->next = MNULL;
    return fb;
}

MadVptr  madFBufferGet(MadFBuffer_t *fb)
{
    MadU8 *res;
    if((MNULL == fb) || (MNULL == fb->head)) {
        return MNULL;
    }
    res = (MadU8*)fb->head;
    fb->head = fb->head->next;
    res += sizeof(MadFBNode_t);
    return res;
}

void  madFBufferPut(MadFBuffer_t *fb, MadVptr buf)
{
    MadFBNode_t *node;
    if((MNULL == fb) || (MNULL == buf)) {
        return;
    }
    node = (MadFBNode_t*)((MadU8*)buf - sizeof(MadFBNode_t));
    node->next = fb->head;
    fb->head = node;
}
