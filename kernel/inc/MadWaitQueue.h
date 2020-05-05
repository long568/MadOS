#ifndef __MAD_WAIT_QUEUE__H__
#define __MAD_WAIT_QUEUE__H__

#include "MadOS.h"

#define MAD_WAITQ_DEFAULT_SIZE 4

enum {
    MAD_WAIT_EVENT_NONE = 0,
    MAD_WAIT_EVENT_READ,
    MAD_WAIT_EVENT_WRITE,
    MAD_WAIT_EVENT_ERR
};

struct _MadWait_t;
struct _MadWaitQ_t;

typedef struct _MadWait_t {
    struct _MadWait_t *next;
    MadSemCB_t **locker;
    MadTCB_t   *thread;
    MadU8      event;
} MadWait_t;

typedef struct _MadWaitQ_t {
    MadWait_t *l0;
    MadWait_t *l1;
} MadWaitQ_t;

extern MadWaitQ_t* madWaitQCreate(MadU8 n);
extern void        madWaitQDelete(MadWaitQ_t *wq);
extern MadBool     madWaitQAdd   (MadWaitQ_t *wq, MadSemCB_t **locker, MadU8 event);
extern MadBool     madWaitQScan  (MadWaitQ_t *wq, MadSemCB_t **locker, MadU8 event, MadWait_t *rw);
extern MadBool     madWaitQSignal(MadWaitQ_t *wq, MadU8 event);

#define madWaitQScanEvent(wq, event, rw)   madWaitQScan(wq,      0, event, rw)
#define madWaitQScanLocker(wq, locker, rw) madWaitQScan(wq, locker,     0, rw)
#define madWaitQRemove(wq, locker, event)  madWaitQScan(wq, locker, event,  0)

#endif
