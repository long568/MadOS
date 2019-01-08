#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "CfgUser.h"
#include "Stm32Tools.h"
#include "MadDrv.h"
#include "MadDrvLora_IntoL6_AT.h"
#include "cJSON.h"
#include "ModO2.h"

#define LORA_RX_RSP      0
#define LORA_TX_INTERVAL (1000 * 28)

#if 1
static StmPIN  lora_led;
#define lora_led_init() do{ lora_led.port = LORA_FLAG_PORT; \
                            lora_led.pin  = LORA_FLAG_PIN;  \
                            StmPIN_DefInitOPP(&lora_led); }while(0)
#define lora_led_on()   StmPIN_SetLow(&lora_led)
#define lora_led_off()  StmPIN_SetHigh(&lora_led)
#else
#define lora_led_init()
#define lora_led_on()
#define lora_led_off() 
#endif

static int     lora_fd;
static MadBool lora_joined;

#if LORA_RX_RSP
static void lora_print_rsp(char *buf);
#endif
static void lora_thread(MadVptr exData);

MadBool ModLora_Init(void)
{
    if(MNULL != madThreadCreate(lora_thread, 0, 2048, THREAD_PRIO_MOD_LORA)) {
        lora_led_init();
        lora_led_off();
        return MTRUE;
    }
    return MFALSE;
}

#if LORA_RX_RSP
static void lora_print_rsp(char *buf)
{
    cJSON *root, *item;
    root = cJSON_Parse(buf);
    if(0 != root) {
        item = cJSON_GetObjectItem(root, "time");
        if(0 != item) {
            MAD_LOG("%s\n", item->valuestring);
        }
        cJSON_Delete(root);
    }
}
#endif

static char * lora_out(void)
{
    char *out;
    char buf[7];
    SensorO2_t o2_data;
    cJSON *root, *item, *array;

    ModO2_GetData(&o2_data);

    root = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "infoList", array=cJSON_CreateArray());

    sprintf(buf, "%d.%d", o2_data.tmp / 100 - 20, o2_data.tmp % 100);
    item = cJSON_CreateObject();
    cJSON_AddStringToObject(item, "title", "温度");
    cJSON_AddStringToObject(item, "value", buf);
    cJSON_AddStringToObject(item, "unit", "℃");
    cJSON_AddItemToArray(array, item);

    sprintf(buf, "%d.%d", o2_data.hum / 100, o2_data.hum % 100);
    item = cJSON_CreateObject();
    cJSON_AddStringToObject(item, "title", "湿度");
    cJSON_AddStringToObject(item, "value", buf);
    cJSON_AddStringToObject(item, "unit", "\%");
    cJSON_AddItemToArray(array, item);

    sprintf(buf, "%d.%d", o2_data.vol / 10, o2_data.vol % 10);
    item = cJSON_CreateObject();
    cJSON_AddStringToObject(item, "title", "噪音");
    cJSON_AddStringToObject(item, "value", buf);
    cJSON_AddStringToObject(item, "unit", "dB");
    cJSON_AddItemToArray(array, item);

    sprintf(buf, "%d.%d", o2_data.o2 / 100, o2_data.o2 % 100);
    item = cJSON_CreateObject();
    cJSON_AddStringToObject(item, "title", "氧气");
    cJSON_AddStringToObject(item, "value", buf);
    cJSON_AddStringToObject(item, "unit", "\%");
    cJSON_AddItemToArray(array, item);

    out=cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    return out;
}

static void lora_thread(MadVptr exData)
{
    int err, err_cnt;
    char *out;
    
    err_cnt = 0;
    lora_joined = MFALSE;
    while(1) {
        if(MFALSE == lora_joined) {
            lora_led_on();
            lora_fd = open("/dev/lora0", 0);
            lora_led_off();
            if (lora_fd > 0) {
                MAD_LOG("Opening lora ... Done\n");
                lora_joined = MTRUE;
                err_cnt = 0;
            } else {
                MAD_LOG("Opening lora ... Failed\n");
            }
        } else {
            out = lora_out();
            err = write(lora_fd, out, strlen(out));
            free(out);

            if(0 > err) {
                err_cnt++;
            } else {
                err_cnt = 0;
            }

            if(err_cnt > 6) {
                close(lora_fd);
                lora_joined = MFALSE;
            } else {
#if LORA_RX_RSP
                do { // Wait for response
                    char buf[64];
                    if(0 < read(lora_fd, buf, 0)) {
                        lora_print_rsp(buf);
                    } else {
                        lora_led_on();
                    }
                } while(0);
#endif
            }

            madTimeDly(LORA_TX_INTERVAL);
            lora_led_off();
        }
    }
}
