#ifndef __MAD_EVENT__H__
#define __MAD_EVENT__H__

#include "MadGlobal.h"

#define MAD_EVENT_TRIGALL  (~((MadUint)0))

typedef enum {
    MEMODE_WAIT_ALL = 0,
    MEMODE_WAIT_ONE
} MadEventMode;

typedef enum {
    MEOPT_DIRECT = 0,
    MEOPT_DELAY
} MadEventOpt;

typedef struct _MadEventCB_t {
    MadU16  rdyg;
    MadU16  rdy[MAD_THREAD_RDY_NUM];
    MadUint maskWait;
    MadUint maskGot;
    MadU8   mode;
    MadU8   opt;
} MadEventCB_t;

extern  MadEventCB_t*  madEventCreate                 (MadUint mask, MadEventMode mode, MadEventOpt opt);
extern  MadU8          madEventWait                   (MadEventCB_t **pEvent, MadUint *mask, MadTime_t to);
extern  MadU8          madEventDoCheck                (MadEventCB_t **pEvent, MadUint *mask, MadBool clear);
extern  void           madDoEventTrigger              (MadEventCB_t **pEvent, MadUint mask, MadU8 err);
extern  void           madDoEventShut                 (MadEventCB_t **pEvent, MadBool opt);
extern  void           madDoEventDelete               (MadEventCB_t **pEvent, MadBool opt);

#define                madEventWaitNR(pEvent, to)     madEventWait(pEvent, to, MNULL)
#define                madEventCheck(pEvent, mask)    madEventDoCheck(pEvent, mask, MTRUE)
#define                madEventCheckNC(pEvent, mask)  madEventDoCheck(pEvent, mask, MFALSE)
#define                madEventClear(pEvent)          madEventDoCheck(pEvent, MNULL, MTRUE)
#define                madEventTrigger(pEvent, mask)  madDoEventTrigger(pEvent, mask, MAD_ERR_OK)
#define                madEventShut(pEvent)           madDoEventShut(pEvent, MTRUE)
#define                madEventDelete(pEvent)         madDoEventDelete(pEvent, MTRUE)

#endif
