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
    cJSON *root, *fmt;
    char *out;

    root=cJSON_CreateObject();	
	cJSON_AddItemToObject(root, "name", cJSON_CreateString("Jack (\"Bee\") Nimble"));
	cJSON_AddItemToObject(root, "format", fmt=cJSON_CreateObject());
	cJSON_AddStringToObject(fmt,"type",		"rect");
	cJSON_AddNumberToObject(fmt,"width",		1920);
	cJSON_AddNumberToObject(fmt,"height",		1080);
	cJSON_AddFalseToObject (fmt,"interlace");
	cJSON_AddNumberToObject(fmt,"frame rate",	24);
    out=cJSON_Print(root);
    cJSON_Delete(root);	
    // free(out);

    lora_joined = MFALSE;
    while(1) {
        if(MFALSE == lora_joined) {
            lora_fd = open("/dev/lora0", 0);
            if (lora_fd > 0) {
                lora_joined = MTRUE;
            }
        } else {
            write(lora_fd, out, strlen(out));
            madTimeDly(LORA_TX_INTERVAL);
        }
    }
}
