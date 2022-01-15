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
    char wbuf[4] = {0};
    char rbuf[4] = {0};

    char *addr = &wbuf[0];
    char *reg  = &wbuf[1];
    char *dat  = &wbuf[2];
    char *rlen = &wbuf[2];

    while(1) {
        dev = open("/dev/i2c", 0);
        if(dev < 0) {
            madTimeDly(3000);
            continue;
        }
        madTimeDly(100);

        // Tx Test
        *addr    = MAX_WRITE;
        *reg     = 0x15;
        *dat     = 0xAA;
        *(dat+1) = 0x55;
        write(dev, wbuf, 4);

        // Rx Test
        *addr = MAX_READ;
        *reg  = 0x15;
        *rlen = 2;
        while (1)
        {
            *rbuf     = 0;
            *(rbuf+1) = 0;
            write(dev, wbuf, 3);
            cnt = read(dev, rbuf, 2);
            if(cnt > 0) {
                __NOP();
                __NOP();
                __NOP();
            }

            madTimeDly(20);
        }

        close(dev);
        madTimeDly(1000);
    }
}
