#ifndef __DAT_STATUS__H__
#define __DAT_STATUS__H__

#define datStatus_STR_LEN   42 // len + dummy
#define datStatus_OP_NUM    12
#define datStatus_OP_STEP   300
#define datStatus_OPNO_NUM  3
#define datStatus_OPNO_STEP (datStatus_OP_STEP / datStatus_OPNO_NUM)

extern void   datStatus_Init   (void);
extern char*  datStatus_Rx2Json(char *buf);
extern char*  datStatus_Json2Tx(char *buf, int len);

#endif
