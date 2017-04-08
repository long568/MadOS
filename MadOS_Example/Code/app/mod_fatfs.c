#include "UserConfig.h"
#include "mod_fatfs.h"

void initMicroSD(void)
{
    static FATFS *fat_MicroSD;
    fat_MicroSD = madMemMalloc(sizeof(FATFS));
    if(fat_MicroSD) {
        if(FR_OK == f_mount(fat_MicroSD, "", 0)) {
            FIL fil;
            UINT bw;
            if(FR_OK == f_open(&fil, "long", FA_CREATE_ALWAYS | FA_WRITE)) {
                f_write(&fil, TEST_STR, strlen(TEST_STR), &bw);
                f_close(&fil);
            }
        } else {
            madMemFreeNull(fat_MicroSD);
        }
    }
}
