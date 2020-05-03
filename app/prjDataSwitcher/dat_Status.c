#include <stdlib.h>
#include <string.h>
#include "MadOS.h"
#include "cJSON.h"
#include "modbus.h"
#include "dat_Status.h"
#include "srv_Modbus.h"

void datStatus_Init(void)
{
}

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

char*  datStatus_Rx2Json(char *buf)
{
    int   i, j;
    char  *p_op, *p_opno, *out;
    cJSON *root, *op, *opno;

    double num;
    int    len;
    char   *tmp;
    char   *str = malloc(datStatus_STR_LEN);
    char   *jobOrderCode = str;
    char   *sn           = str;
    if(!str) return MNULL;

    root = cJSON_CreateArray();
    p_op = buf;
    for(i=0; i<datStatus_OP_NUM; i++) {
        p_opno = p_op;
        op = cJSON_CreateArray();

        for(j=0; j<datStatus_OPNO_NUM; j++) {
            opno = cJSON_CreateObject();
            
            len = p_opno[0];
            tmp = p_opno + 2;
            snprintf(jobOrderCode, len, "%s", tmp);
            cJSON_AddStringToObject(opno, "jobOrderCode", jobOrderCode);
            p_opno += datStatus_STR_LEN;

            len = p_opno[0];
            tmp = p_opno + 2;
            snprintf(sn, len, "%s", tmp);
            cJSON_AddStringToObject(opno, "sn", sn);
            p_opno += datStatus_STR_LEN;

            num = (double)(*(MadU16*)p_opno);
            cJSON_AddNumberToObject(opno, "goodQty", num);
            p_opno += 2;

            num = (double)(*(MadU16*)p_opno);
            cJSON_AddNumberToObject(opno, "badQty", num);
            p_opno += 2;

            num = (double)(*(MadU16*)p_opno);
            cJSON_AddNumberToObject(opno, "agvStatus", num);
            p_opno += 2;

            num = (double)(*(MadU16*)p_opno);
            cJSON_AddNumberToObject(opno, "eqStatus", num);
            p_opno += 2;

            num = (double)(*(MadU16*)p_opno);
            cJSON_AddNumberToObject(opno, "upFlag", num);
            p_opno += 2;

            cJSON_AddItemToArray(op, opno);
            p_opno += 6;
        }

        cJSON_AddItemToArray(root, op);
        p_op += datStatus_OP_STEP;
    }

#if datStatus_USE_AGV
    op = cJSON_CreateArray();
    for(i=0; i<3; i++) {
        num = (double)(*(MadU16*)p_op);
        opno = cJSON_CreateNumber(num);
        cJSON_AddItemToArray(op, opno);
        p_op += 2;
    }
    cJSON_AddItemToArray(root, op);
#endif

    free(str);
    out = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    return out;
}

#define HANDLE_STR(x) do{                   \
    item = cJSON_GetObjectItem(opno, x);    \
    str  = cJSON_GetStringValue(item);      \
    tmp[0] = strlen(str);                   \
    tmp[1] = '(';                           \
    strcpy(tmp+2, str);                     \
    tmp += datStatus_STR_LEN;               \
} while(0)

#define HANDLE_NUM(x) do{                   \
    item = cJSON_GetObjectItem(opno, x);    \
    num  = cJSON_GetNumberValue(item);      \
    *((MadU16*)tmp) = (MadU16)num;          \
    tmp += 2;                               \
} while(0)

char*  datStatus_Json2Tx(char *buf, int len)
{
    int   i, j;
    double num;
    char  *p_op, *p_opno, *out, *tmp, *str;
    cJSON *root, *op, *opno, *item;

    root = cJSON_ParseWithLength(buf, len);
    free(buf);
    if(!root) {
        MAD_LOG("[DatStatus]Parse json failed![%X][%d]\n", (MadUint)buf, len);
        return MNULL;
    }

    out = (char*)malloc(srvModbus_BUFSIZ);
    if(!out) {
        MAD_LOG("[DatStatus]Allocate out failed!\n");
        cJSON_Delete(root);
        return MNULL;
    }

    p_op = out;
    for(i=0; i<datStatus_OP_NUM; i++) {
        op = cJSON_GetArrayItem(root, i);
        p_opno = p_op;
        for(j=0; j<datStatus_OPNO_NUM; j++) {
            opno = cJSON_GetArrayItem(op, j);
            tmp = p_opno;
            HANDLE_STR("jobOrderCode");
            HANDLE_STR("sn");
            HANDLE_NUM("qty");
            HANDLE_NUM("qa");
            HANDLE_NUM("downFlag");
            p_opno += datStatus_OPNO_STEP;
        }
        p_op += datStatus_OP_STEP;
    }

    cJSON_Delete(root);
    return out;
}
