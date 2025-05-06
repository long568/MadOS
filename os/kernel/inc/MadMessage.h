#ifndef __MAD_MESSAGE__H__
#define __MAD_MESSAGE__H__

#include "MadSemaphore.h"

typedef void(*madMsgFree_Callback)(MadVptr p);

typedef struct _MadMsgQCB_t {
    MadU16     rdyg;
    MadU16     rdy[MAD_THREAD_RDY_NUM];
    MadU8      **top;
    MadU8      **bottom;
    MadU8      **head;
    MadU8      **tail;
    MadSemCB_t *sem;
    MadU16     cnt;
    MadU16     size;
} MadMsgQCB_t;

extern  const  MadU8  MAD_MSG_EMPTY[];

extern  MadMsgQCB_t*  madMsgQCreateCarefully           (MadU16 size, MadBool sendBlock);
extern 	void          madMsgQClear                     (MadMsgQCB_t **pMsgQ, madMsgFree_Callback msgFree);
extern  MadU8 		  madMsgCheck                      (MadMsgQCB_t **pMsgQ, MadVptr *msg);
extern  MadU8         madMsgWait                       (MadMsgQCB_t **pMsgQ, MadVptr *msg, MadTime_t to);
extern  MadU8         madDoMsgSend                     (MadMsgQCB_t **pMsgQ, MadVptr msg, MadBool block, MadTime_t to, MadU8 err);
extern  void          madDoMsgQShut                    (MadMsgQCB_t **pMsgQ, MadBool opt);
extern  void 		  madDoMsgQDelete                  (MadMsgQCB_t **pMsgQ, MadBool opt);

#define               madMsgQCreate(size)              madMsgQCreateCarefully(size, MFALSE);
#define               madMsgSend(pMsgQ, msg)           madDoMsgSend(pMsgQ, msg, MFALSE, 0, MAD_ERR_OK)
#define               madMsgSendBlock(pMsgQ, msg, to)  madDoMsgSend(pMsgQ, msg, MTRUE, to, MAD_ERR_OK)
#define               madMsgQShut(pMsgQ)               madDoMsgQShut(pMsgQ, MTRUE)
#define               madMsgQDelete(pMsgQ)             madDoMsgQDelete(pMsgQ, MTRUE)

#endif
