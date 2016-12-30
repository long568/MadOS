#include "MadOS.h"

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
    s_real = ((size % MAD_MEM_ALIGN) ? MAD_MEM_ALIGN : 0) + (size & MAD_MEM_ALIGN_MASK);
    step = sizeof(MadFBNode_t) + s_real;
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

MadVptr madFBufferGet(MadFBuffer_t *fb)
{
    MadCpsr_t cpsr;
    MadU8     *res;
    madEnterCritical(cpsr);
    if((MNULL == fb) || (MNULL == fb->head)) {
        madExitCritical(cpsr);
        return MNULL;
    }
    res = (MadU8*)fb->head;
    fb->head = fb->head->next;
    fb->n--;
    res += sizeof(MadFBNode_t);
    madExitCritical(cpsr);
    return res;
}

void madFBufferPut(MadFBuffer_t *fb, MadVptr buf)
{
    MadCpsr_t   cpsr;
    MadFBNode_t *node;
    madEnterCritical(cpsr);
    if((MNULL == fb) || (MNULL == buf)) {
        madExitCritical(cpsr);
        return;
    }
    node = (MadFBNode_t*)((MadU8*)buf - sizeof(MadFBNode_t));
    node->next = fb->head;
    fb->head = node;
    fb->n++;
    madExitCritical(cpsr);
}
