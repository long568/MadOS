#ifndef __SRV_MQTT__H__
#define __SRV_MQTT__H__

#include "lwip/apps/mqtt.h"
#include "lwip/apps/mqtt_priv.h"
#include "mod_Network.h"

enum {
    MQTT_MSGT_STATION_UP = MQTT_CONNECT_TIMEOUT + 1,
    MQTT_MSGT_STATION_DOWN,
    MQTT_MSGT_AUTOSTORE_UP,
    MQTT_MSGT_AUTOSTORE_DOWN,
    MQTT_MSGT_AGV_UP,
    MQTT_MSGT_AGV_DOWN,
    MQTT_MSGT_UNKNOWN
};

extern MadMsgQCB_t  *MQTT_MsgQ;
extern MadFBuffer_t *MQTT_MsgG;

extern void srvMQTT_Init(void);

#endif
