#ifndef __MAD_FASTBUFFER__H__
#define __MAD_FASTBUFFER__H__

#include "MadGlobal.h"

struct _MadFBNode_t;
struct _MadFBuffer_t;

typedef struct _MadFBNode_t {
    struct _MadFBNode_t *next;
} MadFBNode_t;

typedef struct _MadFBuffer_t {
    MadSize_t   n;
    MadSize_t   max;
    MadFBNode_t *head;
} MadFBuffer_t;

extern  MadFBuffer_t*  madFBufferCreate          (MadSize_t n, MadSize_t size);
extern  MadVptr        madFBufferGet             (MadFBuffer_t *fb);
extern  void           madFBufferPut             (MadFBuffer_t *fb, MadVptr buf);
#define                madFBufferUnusedCount(fb) (fb->n)
#define                madFBufferMaxCount(fb)    (fb->max)
#define                madFBufferDelete(fb)      madMemFree(fb)
#define                madFBufferDeleteNull(fb)  madMemFreeNull(fb)
#define                madFBufferSafeDelete(fb)  madMemSafeFree(fb)

#endif
