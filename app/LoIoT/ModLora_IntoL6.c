#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "CfgUser.h"
#include "Stm32Tools.h"
#include "MadDrv.h"
#include "MadDrvLora_IntoL6_AT.h"
#include "ModO2.h"
#include "ModNH3.h"

#define LORA_RX_RSP      0
#define LORA_TX_INTERVAL 28000 //(1000 * 60 * 1)

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
static void  lora_thread (MadVptr exData);

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

static void lora_thread(MadVptr exData)
{
    int err, err_cnt;
    char *out;
    
    err_cnt = 0;
    lora_joined = MFALSE;
    // madWatchDog_Start(WATCHDOG_3MIN);

    while(1) {
        if(MFALSE == lora_joined) {
            lora_led_on();
            lora_fd = open("/dev/lora0", 0);
            // madWatchDog_Feed();
            lora_led_off();
            if (lora_fd > 0) {
                MAD_LOG("Opening lora ... Done\n");
                lora_joined = MTRUE;
                err_cnt = 0;
            } else {
                MAD_LOG("Opening lora ... Failed\n");
            }
        } else {
            lora_led_on();
            out = ModNH3_GetData();
            err = write(lora_fd, out, strlen(out));
            free(out);
            lora_led_off();

            if(0 > err) {
                err_cnt++;
            } else {
                err_cnt = 0;
            }

            if(err_cnt > 3) {
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

            // madWatchDog_Feed();
            madTimeDly(LORA_TX_INTERVAL);
        }
    }
}
