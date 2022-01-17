#ifndef __LOOP__H__
#define __LOOP__H__

#include "MadOS.h"

typedef enum {
    MSG_KEY,
    MSG_BLE_SYNC,
    MSG_BLE_SLEEP,
    MSG_BLE_HR,
    MSG_BLE_SHUT
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
} msg_t;

extern MadBool loop_init    (void);
extern MadU8   loop_msg_send(MadVptr msg);

#endif
