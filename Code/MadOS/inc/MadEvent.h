#ifndef __MAD_EVENT__H__
#define __MAD_EVENT__H__

#include "inc/MadGlobal.h"

#define MAD_EVENT_TRIGALL  ((MadU16)0xFFFF)

typedef struct _MadEventCB_t {
    MadU16  rdyg;
    MadU16  rdy[MAD_THREAD_RDY_NUM];
    MadU16  maskWait;
    MadU16  maskGot;
	MadU16  cnt;
} MadEventCB_t;

extern  MadEventCB_t*  madEventCreate     (MadU16 mask);
extern  MadU8          madDoEventWait	  (MadEventCB_t **pEvent, MadU16 mask, MadTim_t to, MadBool reset);
extern  MadU8          madEventCheck      (MadEventCB_t **pEvent, MadU16 *mask);
extern  void           madDoEventTrigger  (MadEventCB_t **pEvent, MadU16 mask, MadU8 err);
extern  void           madDoEventDelete   (MadEventCB_t **pEvent, MadBool opt);

#define madEventWait(pEvent, mask, to)    madDoEventWait(pEvent, mask, to, MFALSE)
#define madEventWaitNew(pEvent, mask, to) madDoEventWait(pEvent, mask, to, MTRUE)
#define madEventTrigger(pEvent, mask)     madDoEventTrigger(pEvent, mask, MAD_ERR_OK)
#define madEventDelete(pEvent)		      madDoEventDelete(pEvent, MTRUE)

#endif
