// typedef enum {
// 	FR_OK = 0,				/* (0) Succeeded */
// 	FR_DISK_ERR,			/* (1) A hard error occurred in the low level disk I/O layer */
// 	FR_INT_ERR,				/* (2) Assertion failed */
// 	FR_NOT_READY,			/* (3) The physical drive cannot work */
// 	FR_NO_FILE,				/* (4) Could not find the file */
// 	FR_NO_PATH,				/* (5) Could not find the path */
// 	FR_INVALID_NAME,		/* (6) The path name format is invalid */
// 	FR_DENIED,				/* (7) Access denied due to prohibited access or directory full */
// 	FR_EXIST,				/* (8) Access denied due to prohibited access */
// 	FR_INVALID_OBJECT,		/* (9) The file/directory object is invalid */
// 	FR_WRITE_PROTECTED,		/* (10) The physical drive is write protected */
// 	FR_INVALID_DRIVE,		/* (11) The logical drive number is invalid */
// 	FR_NOT_ENABLED,			/* (12) The volume has no work area */
// 	FR_NO_FILESYSTEM,		/* (13) There is no valid FAT volume */
// 	FR_MKFS_ABORTED,		/* (14) The f_mkfs() aborted due to any problem */
// 	FR_TIMEOUT,				/* (15) Could not get a grant to access the volume within defined period */
// 	FR_LOCKED,				/* (16) The operation is rejected according to the file sharing policy */
// 	FR_NOT_ENOUGH_CORE,		/* (17) LFN working buffer could not be allocated */
// 	FR_TOO_MANY_OPEN_FILES,	/* (18) Number of open files > FF_FS_LOCK */
// 	FR_INVALID_PARAMETER	/* (19) Given parameter is invalid */
// } FRESULT;
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include "MadOS.h"
#include "CfgUser.h"
#include "ff.h"
#include "testFatFs.h"

#define HELLO_MADOS "Hello MadOS\nNow, we are ONE !!!\n"
#define HELLO_LEN   sizeof(HELLO_MADOS)-1
#define BUFF_LEN    HELLO_LEN + 8
#define WRITE_CNT   (10 * 1024 * 1024 / 32)
#define OPT_INTRVAL 5

static void test_fatfs_act(MadVptr exData);

void Init_TestFatFs(void)
{
    madThreadCreate(test_fatfs_act, 0, 1024 * 12, THREAD_PRIO_TEST_FATFS);
}

static void test_fatfs_act(MadVptr exData)
{
    int i, j;

#if 0
    volatile int res;
    UINT cnt;
    FIL *fil;
    MadU8 *buf;

    fil = (FIL*)malloc(sizeof(FIL));
    if(fil == MNULL) {
        MAD_LOG("fil = (FIL*)malloc(sizeof(FIL)) ... failed\n");
        while(1) {
            madTimeDly(UINT32_MAX);
        }
    }

    res = f_open(fil, "/sd/hello.md", FA_OPEN_ALWAYS | FA_READ | FA_WRITE);
    if(res == FR_OK) {
        res = f_write(fil, HELLO_MADOS, HELLO_LEN, &cnt);
        if(res != FR_OK) {
            MAD_LOG("f_write ... failed\n");
        }
        f_close(fil);
        MAD_LOG("f_close ... done\n");
    } else {
        MAD_LOG("f_open ... failed\n");
    }

    res = f_open(fil, "/sd/hello.md", FA_OPEN_EXISTING | FA_READ | FA_WRITE);
    if(res == FR_OK) {
        buf = (MadU8*)malloc(BUFF_LEN);
        res = f_read(fil, buf, BUFF_LEN, &cnt);
        if(res != FR_OK) {
            MAD_LOG("f_read ... failed\n");
        } else {
            MAD_LOG("f_read [%d][%s]\n", cnt, buf);
        }
        f_close(fil);
        MAD_LOG("f_close ... done\n");
        free(buf);
    } else {
        MAD_LOG("f_open ... failed\n");
    }
#endif

#if 1
    FILE *fil;
    size_t cnt;
    MadU8 *buf;
    MadU8 err_code;

    madTimeDly(3000);

    do {
        madTimeDly(100);
        fil = fopen("/sd/hello.md", "w");
        if(fil) {
            cnt = fwrite(HELLO_MADOS, HELLO_LEN, 1, fil);
            fclose(fil);
            fil = 0;
            MAD_LOG("fwrite [%d / %d]\n", cnt, 1);
        } else {
            MAD_LOG("fopen err\n");
        }

        madTimeDly(100);
        fil = fopen("/sd/hello.md", "r");
        if(fil) {
            buf = (MadU8*)malloc(BUFF_LEN);
            cnt = fread(buf, HELLO_LEN, 1, fil);
            fclose(fil);
            fil = 0;
            MAD_LOG("fread [%d][%s]\n", cnt, buf);
            free(buf);
        } else {
            MAD_LOG("fopen err\n");
        }
    } while(0);
#endif

    madThreadPend(MAD_THREAD_SELF);

    i = 0;
    j = 0;
    err_code = 0;
    
    while(1) {
        if(i++ < WRITE_CNT) {
#if 1
            madTimeDly(OPT_INTRVAL);
            fil = fopen("/sd/hello.md", "a");
            if(fil) {
                cnt = fwrite(HELLO_MADOS, HELLO_LEN, 1, fil);
                fclose(fil);
                fil = 0;
                if(cnt != 1) {
                    err_code = 3;
                    i = WRITE_CNT;
                    continue;
                }
            } else {
                err_code = 1;
                i = WRITE_CNT;
                continue;
            }
#endif

#if 1
            madTimeDly(OPT_INTRVAL);
            fil = fopen("/sd/hello.md", "r");
            if(fil) {
                buf = (MadU8*)malloc(BUFF_LEN);
                if(buf) {
                    cnt = fread(buf, HELLO_LEN, 1, fil);
                } else {
                    MAD_LOG("buf malloc failed [%d]\n", i);
                    cnt = 0;
                }
                fclose(fil);
                fil = 0;
                free(buf);
                if(cnt != 1) {
                    err_code = 4;
                    i = WRITE_CNT;
                    continue;
                }
            } else {
                err_code = 2;
                i = WRITE_CNT;
                continue;
            }
#endif

            if(0 == i%1000) {
                // i = 0;
                MAD_LOG("ff working [%d]\n", ++j);
            }
        } else {
            MAD_LOG("ff done [%d / %d]\n", j, err_code);
            madTimeDly(UINT32_MAX);
        }
    }
}
