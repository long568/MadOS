#ifndef __BLE__H__
#define __BLE__H__

#include "MadOS.h"

typedef enum {
    // Base
    BLE_CMD_SYNC = 1,
    BLE_CMD_ES_LEVEL,
    BLE_CMD_PRODUCT_VER,
    BLE_DUMMY_0,
    BLE_CMD_SHUT,
    BLE_CMD_ES_FREQ,
    BLE_CMD_SYS_TOUT,
    BLE_CMD_SYS_TT,
    // ID
    BLE_CMD_ID = 10,
    BLE_CMD_TID,
    BLE_CMD_VERIFY,
    BLE_CMD_CLEAR,
    // Wallet
    BLE_CMD_KEY_W = 20,
    BLE_CMD_KEY_R,
    BLE_CMD_KEY_D,
    BLE_CMD_KEY_L,
    BLE_CMD_KEY_C
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
extern MadU8*  ble_mac_adr(void);

#endif
