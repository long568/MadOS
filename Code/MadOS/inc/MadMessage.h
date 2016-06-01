#ifndef __MAD_MESSAGE__H__
#define __MAD_MESSAGE__H__

#include "inc/MadSemaphore.h"

typedef struct _madMsgQCB_t
{
    mad_u16     rdyg;
    mad_u16     rdy[MAD_THREAD_RDY_NUM];
    mad_u8      **top;
    mad_u8      **bottom;
    mad_u8      **head;
    mad_u8      **tail;
    madSemCB_t  *sem;
    mad_u16     cnt;
    mad_u16     size;
} madMsgQCB_t;

extern  mad_const  mad_u8 MAD_MSG_EMPTY[];

extern  madMsgQCB_t*  madMsgQCreateCarefully  (mad_u16 size, mad_bool_t sendBlock);
extern 	mad_u8 		  madMsgCheck             (madMsgQCB_t **pmsgQ);
extern  mad_u8        madMsgWait              (madMsgQCB_t **pmsgQ, mad_tim_t to);
extern  mad_u8        madDoMsgSend            (madMsgQCB_t **pmsgQ, mad_vptr msg, mad_bool_t block, mad_tim_t to, mad_u8 err);
extern  void 		  madDoMsgQDelete	      (madMsgQCB_t **pmsgQ, mad_bool_t opt);

#define madMsgQCreate(size)              madMsgQCreateCarefully(size, MFALSE);
#define madMsgSend(pmsgQ, msg)           madDoMsgSend(pmsgQ, msg, MFALSE, 0, MAD_ERR_OK)
#define madMsgSendBlock(pmsgQ, msg, to)  madDoMsgSend(pmsgQ, msg, MTRUE, to, MAD_ERR_OK)
#define madMsgQDelete(pmsgQ)			 madDoMsgQDelete(pmsgQ, MTRUE)

#endif
