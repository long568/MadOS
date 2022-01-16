#ifndef __BLE__H__
#define __BLE__H__

#include "MadOS.h"

typedef enum {
    BLE_CMD_SLEEP = 1,
    BLE_CMD_HR,
    BLE_CMD_EQ
} ble_cmd_enum;

typedef struct {
    MadU8 cmd;
    MadU8 len;
    union {
        MadU8 v;
        MadU8 *p;
    } arg;
} ble_cmd_t;

extern MadBool ble_init(void);
extern int     ble_send(const ble_cmd_t *c);

#endif
