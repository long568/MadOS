#include "MadOS.h"
#include "test.h"

const MadU8 haha[] = "Hello world !!";

t_data_t     t_tim[2] = {{1,371,0,0x01,0}, {3,111,0,0x10,0}};
MadUint      t_i;
MadSize_t    t_unused_size;
MadMsgQCB_t  *t_msgQ;
MadEventCB_t *t_event;
MadFBuffer_t *t_fBuf;
