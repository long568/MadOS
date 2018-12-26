#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include "CfgUser.h"
#include "Stm32Tools.h"
#include "MadDrv.h"
#include "MadDrvLora_IntoL6_AT.h"
#include "cJSON.h"

#define LORA_TX_INTERVAL (1000 * 28)

#if 0
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

static void lora_print_rsp(char *buf);
static void lora_thread (MadVptr exData);

MadBool ModLora_Init(void)
{
    if(MNULL != madThreadCreate(lora_thread, 0, 2048, THREAD_PRIO_MOD_LORA)) {
        return MTRUE;
    }
    return MFALSE;
}

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

static void lora_thread(MadVptr exData)
{
    char *out;
    char buf[64];
    
    do {
        cJSON *root, *item, *array;
        root = cJSON_CreateObject();
        cJSON_AddItemToObject(root, "infoList", array=cJSON_CreateArray());

        item = cJSON_CreateObject();
        cJSON_AddStringToObject(item, "title", "用电量");
        cJSON_AddStringToObject(item, "value", "568");
        cJSON_AddStringToObject(item, "unit",  "kWh");
        cJSON_AddItemToArray(array, item);

        item = cJSON_CreateObject();
        cJSON_AddStringToObject(item, "title", "峰值");
        cJSON_AddStringToObject(item, "value", "36");
        cJSON_AddStringToObject(item, "unit",  "℃");
        cJSON_AddItemToArray(array, item);

        out=cJSON_PrintUnformatted(root);
        cJSON_Delete(root);
        // free(out);
    } while(0);

    lora_joined = MFALSE;
    while(1) {
        if(MFALSE == lora_joined) {
            lora_fd = open("/dev/lora0", 0);
            if (lora_fd > 0) {
                MAD_LOG("Open lora device ... OK\n");
                lora_joined = MTRUE;
            } else {
                MAD_LOG("Open lora device ... Failed\n");
            }
        } else {
            write(lora_fd, out, strlen(out));
            if(0 < read(lora_fd, buf, 0)) { // Wait for response
                lora_print_rsp(buf);
            }
            madTimeDly(LORA_TX_INTERVAL);
        }
    }
}
