#ifndef __DAT_STATUS__H__
#define __DAT_STATUS__H__

extern void   datStatus_Init   (void);
extern char*  datStatus_Rx2Json(char *buf);
extern char*  datStatus_Json2Tx(char *json);

#endif
