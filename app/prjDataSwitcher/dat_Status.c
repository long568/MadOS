#include <stdlib.h>
#include <string.h>
#include "MadOS.h"
#include "cJSON.h"
#include "modbus.h"
#include "dat_Status.h"

void datStatus_Init(void)
{
}

// inline
// void datStatus_Clear(void)
// {
//     memset(rxBuff, 0, DAT_STATUS_BUFF_LEN);
// }

// inline
// int datStatus_Lock(void)
// {
//     return (MAD_ERR_OK == madMutexWait(&locker, 0)) ? 1 : 0;
// }

// inline
// void datStatus_UnLock(void)
// {
//     madMutexRelease(&locker);
// }

// inline
// MadU8* datStatus_RxBuff(void)
// {
//     return rxBuff;
// }

// static void switchStr(char *dst, char *src)
// {
//     int i, j;
//     int len = src[0];
//     i = 0;
//     j = len;
//     while(1) {
//         if(j--) dst[i]   = src[i+3]; else break;
//         if(j--) dst[i+1] = src[i+2]; else break;
//         i += 2;
//     }
//     dst[len] = 0;
// }

// char* datStatus_Rx2Json(char *buff)
// {
//     int i;
//     char  *tmp;
//     cJSON *root, *item;
//     char  *out = 0;

//     char   str[DAT_STATUS_STRING_SIZE];
//     char   org[DAT_STATUS_STRING_SIZE];
//     double num;
//     char   *eqCode       = str;
//     char   *jobOrderCode = str;
//     char   *sn           = str;

//     root = cJSON_CreateArray();
//     datStatus_Lock();

//     for(i=0; i<DAT_STATUS_NUM; i++) {
//         tmp  = buff;
//         item = cJSON_CreateObject();

//         sprintf(eqCode, "Eq%03d", i+1);
//         cJSON_AddStringToObject(item, "eqCode", eqCode);

//         switchStr(org, (char*)tmp);
//         snprintf(jobOrderCode, DAT_STATUS_STRING_SIZE, "%s", org);
//         cJSON_AddStringToObject(item, "jobOrderCode", jobOrderCode);
//         tmp += DAT_STATUS_STRING_SIZE;

//         switchStr(org, (char*)tmp);
//         snprintf(sn, DAT_STATUS_STRING_SIZE, "%s", org);
//         cJSON_AddStringToObject(item, "sn", sn);
//         tmp += DAT_STATUS_STRING_SIZE;

//         num = (double)(*(MadU16*)tmp);
//         cJSON_AddNumberToObject(item, "goodQty", num);
//         tmp += 2;

//         num = (double)(*(MadU16*)tmp);
//         cJSON_AddNumberToObject(item, "badQty", num);
//         tmp += 2;

//         num = (double)(*(MadU16*)tmp);
//         cJSON_AddNumberToObject(item, "agvStatus", num);
//         tmp += 2;

//         num = (double)(*(MadU16*)tmp);
//         cJSON_AddNumberToObject(item, "eqStatus", num);
//         tmp += 2;

//         num = (double)(*(MadU16*)tmp);
//         cJSON_AddNumberToObject(item, "upFlag", num);

//         cJSON_AddItemToArray(root, item);
//         buff += 100;
//     }

//     datStatus_UnLock();
//     out = cJSON_PrintUnformatted(root);
//     cJSON_Delete(root);
//     return out;
// }

char*  datStatus_Rx2Json(char *buf)
{
    return buf;
}

char*  datStatus_Json2Tx(char *json)
{
    return json;
}
