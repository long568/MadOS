#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include "CfgUser.h"
#include "cJSON.h"
#include "MadDrvO2.h"

static int o2_fd;
static int o2_opened = MFALSE;
static MadSemCB_t *o2_locker;
static SensorO2_t o2_data = { 0, 0, 0, 0 };

static void o2_log        (void);
static void o2_zero_data  (void);
static void o2_handle_data(SensorO2_t *data);
static void o2_thread     (MadVptr exData);

inline static void o2_lock(void)   { madSemWait(&o2_locker, 0); }
inline static void o2_unlock(void) { madSemRelease(&o2_locker); }

MadBool ModO2_Init(void)
{
    o2_locker = madSemCreate(1);
    if(MNULL == o2_locker) return MFALSE;
    if(MNULL == madThreadCreate(o2_thread, 0, 2048, THREAD_PRIO_MOD_O2)) {
        madSemDelete(&o2_locker);
        return MFALSE;
    }
    return MTRUE;
}

char* ModO2_GetData(void)
{
    char *out;  // Freed by user
    char buf[7];
    SensorO2_t data;
    cJSON *root, *item, *array;

    if(!o2_opened) {
        data.tmp = 0;
        data.hum = 0;
        data.vol = 0;
        data.o2  = 0;
    } else {
        o2_lock();
        data.tmp = o2_data.tmp;
        data.hum = o2_data.hum;
        data.vol = o2_data.vol;
        data.o2  = o2_data.o2;
        o2_unlock();
    }

    root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "type", "sensor");
    cJSON_AddItemToObject(root, "data", array=cJSON_CreateArray());

    sprintf(buf, "%d.%d", data.tmp / 100 - 20, data.tmp % 100);
    item = cJSON_CreateObject();
    cJSON_AddStringToObject(item, "t", "温度");
    cJSON_AddStringToObject(item, "v", buf);
    cJSON_AddStringToObject(item, "u", "℃");
    cJSON_AddItemToArray(array, item);

    sprintf(buf, "%d.%d", data.hum / 100, data.hum % 100);
    item = cJSON_CreateObject();
    cJSON_AddStringToObject(item, "t", "湿度");
    cJSON_AddStringToObject(item, "v", buf);
    cJSON_AddStringToObject(item, "u", "\%");
    cJSON_AddItemToArray(array, item);

    sprintf(buf, "%d.%d", data.vol / 10, data.vol % 10);
    item = cJSON_CreateObject();
    cJSON_AddStringToObject(item, "t", "噪音");
    cJSON_AddStringToObject(item, "v", buf);
    cJSON_AddStringToObject(item, "u", "dB");
    cJSON_AddItemToArray(array, item);

    sprintf(buf, "%d.%d", data.o2 / 100, data.o2 % 100);
    item = cJSON_CreateObject();
    cJSON_AddStringToObject(item, "t", "氧气");
    cJSON_AddStringToObject(item, "v", buf);
    cJSON_AddStringToObject(item, "u", "\%");
    cJSON_AddItemToArray(array, item);

    out=cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    return out;
}

static void o2_log(void)
{
    MAD_LOG("Sensor O2:%04X-%04X-%04X-%04X\n",
        o2_data.tmp, o2_data.hum, o2_data.vol, o2_data.o2);
}

static void o2_zero_data(void)
{
    o2_lock();
    o2_data.tmp = 0;
    o2_data.hum = 0;
    o2_data.vol = 0;
    o2_data.o2  = 0;
    o2_unlock();
    o2_log();
}

static void o2_handle_data(SensorO2_t *data)
{
    o2_lock();
    if(o2_data.vol == 0) {
        o2_data.tmp = data->tmp;
        o2_data.hum = data->hum;
        o2_data.vol = data->vol;
        o2_data.o2  = data->o2;
    } else {
        o2_data.tmp = (o2_data.tmp + data->tmp) / 2;
        o2_data.hum = (o2_data.hum + data->hum) / 2;
        o2_data.vol = (o2_data.vol + data->vol) / 2;
        o2_data.o2  = (o2_data.o2  + data->o2 ) / 2;
    }
    o2_unlock();
    o2_log();
}

static void o2_thread(MadVptr exData)
{
    int res;
    SensorO2_t data;
    
    while(1) {
        if(o2_opened) {
            madTimeDly(3000);
            res = read(o2_fd, &data, 0);
            if(0 > res) {
                close(o2_fd);
                o2_zero_data();
                o2_opened = MFALSE;
            } else {
                o2_handle_data(&data);
            }
        } else {
            o2_fd = open("/dev/o20", 0);
            if(o2_fd > 0) {
                o2_opened = MTRUE;
            }
        }
    }
}
