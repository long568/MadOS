#include <fcntl.h>
#include <unistd.h>
#include "CfgUser.h"
#include "heartrate.h"

#define MAX_ADDR   0x57
#define MAX_READ   ((MAX_ADDR << 1) | 0x01)
#define MAX_WRITE  ((MAX_ADDR << 1) & 0xFE)

static void hr_handler(MadVptr exData);

MadBool hr_init(void)
{
    madThreadCreate(hr_handler, 0, 256, THREAD_PRIO_HR);
    return MTRUE;
}

/*
 * buf[0] = ADDR7 + R/Wn
 * buf[1] = reg
 * buf[2] = w_dat / r_len
 */
static void hr_handler(MadVptr exData)
{
    int cnt;
    int dev;
    char buf[4];

    char *addr  = &buf[0];
    char *reg   = &buf[1];
    char *dat   = &buf[2];
    char *r_len = &buf[2];

    while(1) {
        dev = open("/dev/i2c", 0);
        if(dev < 0) {
            madTimeDly(3000);
            continue;
        }
        madTimeDly(100);

#if 1
        // Tx Test
        *addr    = MAX_WRITE;
        *reg     = 0x15;
        *dat     = 0xAA;
        *(dat+1) = 0x55;
        write(dev, buf, 4);
        // while(1) {
        //     write(dev, buf, 3);
        //     madTimeDly(10);
        // }

        // Rx Test
        *addr  = MAX_READ;
        *reg   = 0x15;
        *r_len = 2;
        write(dev, buf, 3);
        cnt = read(dev, dat, 2);
        if(cnt > 0) {
            __NOP();
            __NOP();
            __NOP();
        }
#endif

        while (1) {
            madTimeDly(3000);
        }

        close(dev);
        madTimeDly(1000);
    }
}
