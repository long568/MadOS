#ifndef __CLOUD__H__
#define __CLOUD__H__

#include "network.h"
#include "fatfs.h"

#define CLOUD_USE_HEARTBEAT 1
#define CLOUD_BUFFER_SIZE   LWIP_BUFFER_SIZE

#define CMD_HEAD_LEN        (8)
#define CMD_HEAD_HEAD       ((char)0xA5)
#define CMD_HEAD_TAIL       ((char)0x5A)
#define CMD_HEAD_BIT_ACT    (2)
#define CMD_HEAD_ACT_ID     ((char)0x01)
#define CMD_HEAD_ACT_TX     ((char)0x02)
#define CMD_HEAD_BIT_ID     (3)
#define CMD_HEAD_ID_BOSS    ((char)0x01)
#define CMD_HEAD_ID_WORKER  ((char)0x02)
#define CMD_HEAD_BIT_CNTL   (3)
#define CMD_HEAD_BIT_CNTH   (4)

extern void cloud(MadVptr exData);
extern void cloudLinkUp(void);
extern void cloudLinkDown(void);

#endif
