#ifndef __BLE__H__
#define __BLE__H__

#include "MadOS.h"

typedef enum {
    // Base
    BLE_CMD_SYNC = 1,
    BLE_CMD_SLEEP,
    BLE_CMD_HR,
    BLE_CMD_SPO2,
    BLE_CMD_SHUT,
    // ID
    BLE_CMD_ID = 10,
    BLE_CMD_TID,
    BLE_CMD_VERIFY,
    // Wallet
    BLE_CMD_KEY_W = 20,
    BLE_CMD_KEY_R,
    BLE_CMD_KEY_D
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
