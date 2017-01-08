#include "fatfs.h"

extern int madLuaMain (int argc, char **argv);

void madLuaThread(MadVptr data);

MadBool initMicroSD(void)
{
    FIL fil;
    UINT bw;
    MadBool res;
    static FATFS *fat_MicroSD;
    res = MFALSE;
    fat_MicroSD = madMemMalloc(sizeof(FATFS));
    if(fat_MicroSD) {
        if(FR_OK == f_mount(fat_MicroSD, "", 0)) {
            if(FR_OK == f_open(&fil, "long", FA_CREATE_ALWAYS | FA_WRITE)) {
                f_write(&fil, TEST_STR, strlen(TEST_STR), &bw);
                f_close(&fil);
                if(madThreadCreate(madLuaThread, 0,  LUA_THREAD_STK_SIZE, THREAD_PRIO_LUA)) {
                    res = MTRUE;
                }
            }
        } else {
            madMemFreeNull(fat_MicroSD);
        }
    }
    return res;
}

void madLuaThread(MadVptr data)
{
    const MadU8 name[]   = "Lua";
    const MadU8 *args[2] = {name, MNULL};
    (void)data;
    while(1) {
        madLuaMain(1, (char **)args);
        madTimeDly(3000);
    }
}
