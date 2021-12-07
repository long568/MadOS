#include <string.h>
#include <stdlib.h>

#include "srv_MQTT.h"
#include "srv_MQTT_Topic.h"

#define MQTT_MSGQ_NUM 8

typedef struct {
    MadU8  id;
    MadU32 tot_len;
    MadU32 cnt;
    MadU8  *buf;
} mqtt_incoming_data_t;

MadMsgQCB_t  *MQTT_MsgQ = MNULL;
MadFBuffer_t *MQTT_MsgG = MNULL;

static mqtt_client_t *mqtt_client;

static void mqtt_thread(MadVptr exData);
static void mqtt_send_msg(MadU16 type, MadU32 len, MadU8 *buf);
static void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status);
static void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len);
static void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags);
static void mqtt_sub_request_cb(void *arg, err_t result);

void srvMQTT_Init(void)
{
    mqtt_client = mqtt_client_new();
    if(!mqtt_client) {
        MAD_LOG("[MQTT]Create client failed\n");
        goto Failed;
        return;
    }

    MQTT_MsgQ = madMsgQCreate(MQTT_MSGQ_NUM);
    MQTT_MsgG = madFBufferCreate(MQTT_MSGQ_NUM, sizeof(mqtt_msg_t));
    if(!MQTT_MsgQ || !MQTT_MsgG) {
        MAD_LOG("[MQTT]Create msgQ failed[%X][%X]\n", (MadU32)MQTT_MsgQ, (MadU32)MQTT_MsgG);
        goto Failed;
        return;
    }

    if(!madThreadCreate(mqtt_thread, MNULL, 1024 * 2, THREAD_PRIO_SRV_MQTT)) {
        MAD_LOG("[MQTT]Create mqtt_thread failed\n");
        goto Failed;
    }

    return;

Failed:
    mqtt_client_free(mqtt_client);
    madMsgQDelete(&MQTT_MsgQ);
    madFBufferDelete(MQTT_MsgG);
    MQTT_MsgQ = MNULL;
    MQTT_MsgG = MNULL;
}

static void mqtt_thread(MadVptr exData)
{
    struct mqtt_connect_client_info_t ci;
    ip_addr_t  srv_addr;
    err_t      err;
    mqtt_msg_t *msg;
    MadU8      go;
    mqtt_incoming_data_t in_data;

    memset(&ci, 0, sizeof(ci));
    ci.client_id = "YCSD_Bosch_Switcher";
    ci.keep_alive = 30;
    IP4_ADDR(&srv_addr, 47, 92, 39, 234);

    while(1) {
        madTimeDly(3000);

        if(!EthIf || !netif_is_link_up(EthIf)) {
            continue;
        }

        err = mqtt_client_connect(mqtt_client, &srv_addr, 1883, mqtt_connection_cb, 0, &ci);
        if(ERR_OK != err) {
            MAD_LOG("[MQTT]Connect ERR[%d]\n", err);
            continue;
        }

        go = MTRUE;
        while(go) {
            madMsgWait(&MQTT_MsgQ, (void**)&msg, 0);
            
            switch (msg->type) {
                case MQTT_CONNECT_ACCEPTED: {
                    MAD_LOG("[MQTT]Connected\n");
                    mqtt_set_inpub_callback(mqtt_client, 
                                            mqtt_incoming_publish_cb,
                                            mqtt_incoming_data_cb, 
                                            &in_data);
                    mqtt_subscribe(mqtt_client, TOPIC_X_DOWN, 2, mqtt_sub_request_cb, TOPIC_X_DOWN);
                    break;
                }

                case MQTT_CONNECT_REFUSED_PROTOCOL_VERSION:
                case MQTT_CONNECT_REFUSED_IDENTIFIER:
                case MQTT_CONNECT_REFUSED_SERVER:
                case MQTT_CONNECT_REFUSED_USERNAME_PASS:
                case MQTT_CONNECT_REFUSED_NOT_AUTHORIZED_:
                case MQTT_CONNECT_DISCONNECTED:
                case MQTT_CONNECT_TIMEOUT: {
                    MAD_LOG("[MQTT]Connect failed[%d]\n", msg->type);
                    mqtt_disconnect(mqtt_client);
                    memset(mqtt_client, 0, sizeof(mqtt_client_t));
                    go = MFALSE;
                    break;
                }

                case MQTT_MSGT_STATION_UP: {
                    break;
                }

                // case MQTT_MSGT_STATION_DOWN: {
                //     break;
                // }

                case MQTT_MSGT_AUTOSTORE_UP: {
                    break;
                }

                // case MQTT_MSGT_AUTOSTORE_DOWN: {
                //     break;
                // }

                case MQTT_MSGT_AGV_UP: {
                    break;
                }

                case MQTT_MSGT_STATION_DOWN:
                case MQTT_MSGT_AUTOSTORE_DOWN:
                case MQTT_MSGT_AGV_DOWN: {
                    if(msg->len > 256) {
                        MAD_LOG("[MQTT]Down[%d]\n", msg->len);
                    } else {
                        MAD_LOG("[MQTT]Down[%d] '%s'\n", msg->len, msg->data);
                    }
                    break;
                }

                default: break;
            }

            free(msg->data);
            madFBufferPut(MQTT_MsgG, msg);
        }
    }
}

static void mqtt_send_msg(MadU16 type, MadU32 len, MadU8 *buf)
{
    mqtt_msg_t *msg;
    msg = madFBufferGet(MQTT_MsgG);
    if(!msg) {
        MAD_LOG("[MQTT]Create msg failed[%d]\n", msg->type);
        return;
    }
    msg->type = type;
    msg->len  = len;
    msg->data = buf;
    if(MAD_ERR_OK != madMsgSend(&MQTT_MsgQ, msg)) {
        MAD_LOG("[MQTT]Send msg failed[%d]\n", msg->type);
        free(msg->data);
        madFBufferPut(MQTT_MsgG, msg);
    }
}

static void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status)
{
    mqtt_send_msg(status, 0, MNULL);
}

static void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len)
{
    MadU8 id;
    mqtt_incoming_data_t *in_data = (mqtt_incoming_data_t*)arg;
    // MAD_LOG("[MQTT]Inpub[%s][%u]\n", topic, (unsigned int)tot_len);
    if(0 == strcmp(topic, TOPIC_STATION_DOWN)) {
        id = TOPIC_ID_STATION;
    } else if(0 == strcmp(topic, TOPIC_AUTOSTORE_DOWN)) {
        id = TOPIC_ID_AUTOSTORE;
    } else if(0 == strcmp(topic, TOPIC_AGV_DOWN)) {
        id = TOPIC_ID_AGV;
    } else {
        id = TOPIC_ID_UNKNOWN;
    }
    in_data->id      = id;
    in_data->tot_len = tot_len;
    in_data->cnt     = 0;
    in_data->buf     = MNULL;
    if(id != TOPIC_ID_UNKNOWN && tot_len > 0) {
        in_data->buf = (MadU8*)malloc(tot_len + 1);
    }
    if(in_data->buf) {
        in_data->buf[tot_len] = 0;
    } else {
        MAD_LOG("[MQTT]in_data->buf == 0\n");
    }
}

static void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags)
{
    MadU8 *buf;
    mqtt_incoming_data_t *in_data = (mqtt_incoming_data_t*)arg;
    // MAD_LOG("[MQTT]Indat[%u]\n", (unsigned int)len);
    if(!in_data->buf) return;
    buf = in_data->buf + in_data->cnt;
    memcpy(buf, data, len);
    in_data->cnt += len;
    if(flags & MQTT_DATA_FLAG_LAST) {
        MadU8  id = in_data->id;
        MadU16 type;
        switch (id) {
            case TOPIC_ID_STATION:   type = MQTT_MSGT_STATION_DOWN;   break;
            case TOPIC_ID_AUTOSTORE: type = MQTT_MSGT_AUTOSTORE_DOWN; break;
            case TOPIC_ID_AGV:       type = MQTT_MSGT_AGV_DOWN;       break;
            default:                 type = MQTT_MSGT_UNKNOWN;        break;
        }
        mqtt_send_msg(type, in_data->cnt, in_data->buf);
    }
}

static void mqtt_sub_request_cb(void *arg, err_t result)
{
    if(ERR_OK == result) {
        MAD_LOG("[MQTT]Sub OK '%s'\n", (char*)arg);
    } else {
        MAD_LOG("[MQTT]Sub ER '%s'\n", (char*)arg);
    }
}
