#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include "CfgUser.h"
#include "cJSON.h"
#include "MadDrvNH3.h"

static int nh3_fd;
static int nh3_opened = MFALSE;
static MadSemCB_t *nh3_locker;
static SensorNH3_t nh3_data = { 0, 0, 0 };

static void nh3_log        (void);
static void nh3_zero_data  (void);
static void nh3_handle_data(SensorNH3_t *data);
static void nh3_thread     (MadVptr exData);

inline static void nh3_lock(void)   { madSemWait(&nh3_locker, 0); }
inline static void nh3_unlock(void) { madSemRelease(&nh3_locker); }

MadBool ModNH3_Init(void)
{
    nh3_locker = madSemCreate(1);
    if(MNULL == nh3_locker) return MFALSE;
    if(MNULL == madThreadCreate(nh3_thread, 0, 2048, THREAD_PRIO_MOD_NH3)) {
        madSemDelete(&nh3_locker);
        return MFALSE;
    }
    return MTRUE;
}

char* ModNH3_GetData(void)
{
    char *out; // Freed by user
    char buf[7];
    SensorNH3_t data;
    cJSON *root, *item, *array;

    if(MFALSE == nh3_opened) {
        data.tmp = 0;
        data.hum = 0;
        data.nh3 = 0;
    } else {
        nh3_lock();
        data.tmp = nh3_data.tmp;
        data.hum = nh3_data.hum;
        data.nh3 = nh3_data.nh3;
        nh3_unlock();
    }

    root = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "infoList", array=cJSON_CreateArray());

    sprintf(buf, "%d.%d", data.tmp / 10, data.tmp % 10);
    item = cJSON_CreateObject();
    cJSON_AddStringToObject(item, "title", "温度");
    cJSON_AddStringToObject(item, "value", buf);
    cJSON_AddStringToObject(item, "unit", "℃");
    cJSON_AddItemToArray(array, item);

    sprintf(buf, "%d.%d", data.hum / 10, data.hum % 10);
    item = cJSON_CreateObject();
    cJSON_AddStringToObject(item, "title", "湿度");
    cJSON_AddStringToObject(item, "value", buf);
    cJSON_AddStringToObject(item, "unit", "\%");
    cJSON_AddItemToArray(array, item);

    sprintf(buf, "%d.%d", data.nh3 / 10, data.nh3 % 10);
    item = cJSON_CreateObject();
    cJSON_AddStringToObject(item, "title", "氨气");
    cJSON_AddStringToObject(item, "value", buf);
    cJSON_AddStringToObject(item, "unit", "ppm");
    cJSON_AddItemToArray(array, item);

    out=cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    return out;
}

static void nh3_log(void)
{
    MAD_LOG("Sensor NH3:%04X-%04X-%04X\n",
        nh3_data.tmp, nh3_data.hum, nh3_data.nh3);
}

static void nh3_zero_data(void)
{
    nh3_lock();
    nh3_data.tmp = 0;
    nh3_data.hum = 0;
    nh3_data.nh3 = 0;
    nh3_unlock();
    nh3_log();
}

static void nh3_handle_data(SensorNH3_t *data)
{
    nh3_lock();
    if(nh3_data.hum == 0) {
        nh3_data.tmp = data->tmp;
        nh3_data.hum = data->hum;
        nh3_data.nh3 = data->nh3;
    } else {
        nh3_data.tmp = (nh3_data.tmp + data->tmp) / 2;
        nh3_data.hum = (nh3_data.hum + data->hum) / 2;
        nh3_data.nh3 = (nh3_data.nh3 + data->nh3) / 2;
    }
    nh3_unlock();
    nh3_log();
}

static void nh3_thread(MadVptr exData)
{
    int res;
    SensorNH3_t data;
    
    while(1) {
        if(nh3_opened) {
            madTimeDly(3000);
            res = read(nh3_fd, &data, 0);
            if(0 > res) {
                close(nh3_fd);
                nh3_zero_data();
                nh3_opened = MFALSE;
            } else {
                nh3_handle_data(&data);
            }
        } else {
            nh3_fd = open("/dev/nh30", 0);
            if(nh3_fd > 0) {
                nh3_opened = MTRUE;
            }
        }
    }
}
