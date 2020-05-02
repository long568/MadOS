#ifndef __DAT_STATUS__H__
#define __DAT_STATUS__H__

// #define DAT_STATUS_LEN         100
// #define DAT_STATUS_NUM         12
// #define DAT_STATUS_STRING_SIZE 42
// #define DAT_STATUS_BUFF_LEN    (DAT_STATUS_LEN * DAT_STATUS_NUM)

extern void   datStatus_Init  (void);
// extern void   datStatus_Clear (void);
// extern int    datStatus_Lock  (void);
// extern void   datStatus_UnLock(void);
// extern MadU8* datStatus_RxBuff(void);
extern char*  datStatus_Rx2Json(char *buf);

#endif
