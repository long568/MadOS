#ifndef __LOOP__H__
#define __LOOP__H__

#include "MadOS.h"

typedef enum {
    MSG_KEY,
    MSG_BLE_SYNC,
    MSG_BLE_ES_LEVEL,
    MSG_BLE_PRODUCT_VER,
    MSG_BLE_SHUT,
    MSG_BLE_ES_FREQ,
    MSG_BLE_SYS_TOUT,
    MSG_BLE_SYS_TT,
    MSG_BLE_CLEAR,
    MSG_BLE_ID,
    MSG_BLE_TID,
    MSG_BLE_VERIFY,
    MSG_BLE_KEY_W,
    MSG_BLE_KEY_R,
    MSG_BLE_KEY_D,
    MSG_BLE_KEY_L,
    MSG_BLE_KEY_C,
    MSG_BLE_NUM
} msg_type_t;

typedef enum {
    MSG_KEY_SHORT,
    MSG_KEY_LONG
} msg_key_arg_t;

typedef struct __msg_t {
    msg_type_t type;
    union {
        MadU8 v;
        MadU8 *p;
    } arg;
    MadU8 len;
} msg_t __attribute__((aligned (4)));

#define SYS_TOUT_DFT    15
#define SYS_TOUT_CNT(x) (MadTime_t)((x) * 60 * 1000)

extern MadBool loop_init    (void);
extern MadU8   loop_msg_send(MadVptr msg);

#endif
