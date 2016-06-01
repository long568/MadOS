#ifndef __MAD_EVENT__H__
#define __MAD_EVENT__H__

#include "inc/MadGlobal.h"

#define MAD_EVENT_TRIGALL  ((mad_u16)0xFFFF)

typedef struct _madEventCB_t
{
    mad_u16  rdyg;
    mad_u16  rdy[MAD_THREAD_RDY_NUM];
    mad_u16  maskWait;
    mad_u16  maskGot;
	mad_u16  cnt;
} madEventCB_t;

extern  madEventCB_t*  madEventCreate     (mad_u16 mask);
extern  mad_u8         madDoEventWait	  (madEventCB_t **pevent, mad_u16 mask, mad_tim_t to, mad_bool_t reset);
extern  mad_u8         madEventCheck      (madEventCB_t **pevent, mad_u16 *mask);
extern  void           madDoEventTrigger  (madEventCB_t **pevent, mad_u16 mask, mad_u8 err);
extern  void           madDoEventDelete   (madEventCB_t **pevent, mad_bool_t opt);

#define madEventWait(pevent, mask, to)    madDoEventWait(pevent, mask, to, MFALSE)
#define madEventWaitNew(pevent, mask, to) madDoEventWait(pevent, mask, to, MTRUE)
#define madEventTrigger(pevent, mask)     madDoEventTrigger(pevent, mask, MAD_ERR_OK)
#define madEventDelete(pevent)		      madDoEventDelete(pevent, MTRUE)

#endif
