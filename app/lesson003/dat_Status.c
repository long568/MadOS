#include <stdlib.h>
#include <string.h>
#include "MadOS.h"
#include "cJSON.h"
#include "dat_Status.h"

static MadMutexCB_t *locker = 0;
static MadU8        *rxBuff = 0;

void datStatus_Init(void)
{
    locker = madMutexCreate();
    rxBuff = malloc(DAT_STATUS_BUFF_LEN);
    memset(rxBuff, 0, DAT_STATUS_BUFF_LEN);
}

inline
int datStatus_Lock(void)
{
    return (MAD_ERR_OK == madMutexWait(&locker, 0)) ? 1 : 0;
}

inline
void datStatus_UnLock(void)
{
    madMutexRelease(&locker);
}

inline
MadU8* datStatus_RxBuff(void)
{
    return rxBuff;
}

char* datStatus_RxJson(void)
{
    int i;
    MadU8 *buff, *tmp;
    cJSON *root, *item;
    char  *out = 0;

    char   str[DAT_STATUS_STRING_SIZE];
    double num;
    char   *eqCode       = str;
    char   *jobOrderCode = str;
    char   *sn           = str;

    root = cJSON_CreateArray();
    datStatus_Lock();

    buff = rxBuff;
    for(i=0; i<DAT_STATUS_NUM; i++) {
        buff += i * 100;
        tmp  = buff;
        item = cJSON_CreateObject();

        sprintf(eqCode, "Eq%03d", i+1);
        cJSON_AddStringToObject(item, "eqCode", eqCode);

        snprintf(jobOrderCode, DAT_STATUS_STRING_SIZE, "%s", tmp);
        cJSON_AddStringToObject(item, "jobOrderCode", jobOrderCode);
        tmp += DAT_STATUS_STRING_SIZE;

        snprintf(sn, DAT_STATUS_STRING_SIZE, "%s", tmp);
        cJSON_AddStringToObject(item, "sn", sn);
        tmp += DAT_STATUS_STRING_SIZE;

        num = (double)(*(MadU16*)tmp);
        cJSON_AddNumberToObject(item, "goodQty", num);
        tmp += 2;

        num = (double)(*(MadU16*)tmp);
        cJSON_AddNumberToObject(item, "badQty", num);
        tmp += 2;

        num = (double)(*(MadU16*)tmp);
        cJSON_AddNumberToObject(item, "agvStatus", num);
        tmp += 2;

        num = (double)(*(MadU16*)tmp);
        cJSON_AddNumberToObject(item, "eqStatus", num);
        tmp += 2;

        num = (double)(*(MadU16*)tmp);
        cJSON_AddNumberToObject(item, "upFlag", num);

        cJSON_AddItemToArray(root, item);
    }

    datStatus_UnLock();
    out = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    return out;
}
