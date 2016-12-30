#ifndef __MAD_EVENT__H__
#define __MAD_EVENT__H__

#include "inc/MadGlobal.h"

#define MAD_EVENT_TRIGALL  (~((MadUint)0))

typedef struct _MadEventCB_t {
    MadU16  rdyg;
    MadU16  rdy[MAD_THREAD_RDY_NUM];
    MadUint maskWait;
    MadUint maskGot;
	MadUint cnt;
} MadEventCB_t;

extern  MadEventCB_t*  madEventCreate                 (MadUint mask);
extern  MadU8          madEventWait	                  (MadEventCB_t **pEvent, MadTim_t to);
extern  MadU8          madEventCheck                  (MadEventCB_t **pEvent, MadUint *mask);
extern  void           madDoEventTrigger              (MadEventCB_t **pEvent, MadUint mask, MadU8 err);
extern  void           madDoEventDelete               (MadEventCB_t **pEvent, MadBool opt);
#define                madEventTrigger(pEvent, mask)  madDoEventTrigger(pEvent, mask, MAD_ERR_OK)
#define                madEventDelete(pEvent)		  madDoEventDelete(pEvent, MTRUE)

#endif
