#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include "CfgUser.h"
#include "Stm32Tools.h"
#include "MadDrvO2.h"

static int o2_fd;
static int o2_opened;
static MadSemCB_t *o2_locker;
static SensorO2_t o2_data = { 0, 0, 0, 0 };

static void o2_thread (MadVptr exData);

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

void ModO2_GetData(SensorO2_t *data)
{
    o2_lock();
    data->tmp = o2_data.tmp;
    data->hum = o2_data.hum;
    data->vol = o2_data.vol;
    data->o2  = o2_data.o2;
    o2_unlock();
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
    MAD_LOG("Sensor:%04X-%04X-%04X-%04X\n",
        o2_data.tmp, o2_data.hum, o2_data.vol, o2_data.o2);
}

static void o2_thread(MadVptr exData)
{
    int res;
    SensorO2_t data;
    o2_opened = MFALSE;
    
    while(1) {
        if(o2_opened) {
            res = read(o2_fd, &data, 0);
            if(0 > res) {
                close(o2_fd);
                o2_opened = MFALSE;
            }
            o2_handle_data(&data);
            madTimeDly(3000);
        } else {
            o2_fd = open("/dev/o20", 0);
            if(o2_fd > 0) o2_opened = MTRUE;
            // madTimeDly(1000 * 60 * 5);
            madTimeDly(1000 * 5);
        }
    }
}
