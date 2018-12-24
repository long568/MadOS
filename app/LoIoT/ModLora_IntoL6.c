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

static void lora_thread (MadVptr exData);

MadBool ModLora_Init(void)
{
    if(MNULL != madThreadCreate(lora_thread, 0, 2048, THREAD_PRIO_MOD_LORA)) {
        return MTRUE;
    }
    return MFALSE;
}

static void lora_thread(MadVptr exData)
{
    cJSON *root, *item, *array;
    char *out;
    
    do {
        root = cJSON_CreateObject();
        cJSON_AddItemToObject(root, "infoList", array=cJSON_CreateArray());

        item = cJSON_CreateObject();
        cJSON_AddStringToObject(item, "title", "用电量");
        cJSON_AddStringToObject(item, "value", "666");
        cJSON_AddStringToObject(item, "unit",  "KWH");
        cJSON_AddItemToArray(array, item);

        item = cJSON_CreateObject();
        cJSON_AddStringToObject(item, "title", "峰值");
        cJSON_AddStringToObject(item, "value", "888");
        cJSON_AddStringToObject(item, "unit",  "℃");
        cJSON_AddItemToArray(array, item);

        out=cJSON_PrintUnformatted(root);
        cJSON_Delete(root);
        // free(out);
    } while(0);

    lora_led_init();
    lora_led_off();
    lora_joined = MFALSE;
    while(1) {
        if(MFALSE == lora_joined) {
            lora_led_on();
            lora_fd = open("/dev/lora0", 0);
            lora_led_off();
            if (lora_fd > 0) {
                lora_joined = MTRUE;
            }
        } else {
            lora_led_on();
            write(lora_fd, out, strlen(out));
            lora_led_off();
            madTimeDly(LORA_TX_INTERVAL);
        }
    }
}
