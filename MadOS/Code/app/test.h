#ifndef __TEST_H__
#define __TEST_H__

typedef struct {
    MadTim_t  t;
    MadSize_t s;
    MadUint   c;
    MadUint   mask;
} t_data_t;

extern const MadU8  haha[];
extern t_data_t     t_tim[2];
extern MadUint      t_i;
extern MadSize_t    t_unused_size;
extern MadMsgQCB_t  *t_msgQ;
extern MadEventCB_t *t_event;
extern MadFBuffer_t *t_fBuf;

extern void initTestMemory(void);
extern void initTestMsgQ(void);
extern void initTestEvent(void);
extern void initTestFB(void);

#endif
