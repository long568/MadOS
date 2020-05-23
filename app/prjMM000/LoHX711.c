#include <fcntl.h>
#include <unistd.h>
#include "pt.h"
#include "timer.h"
#include "Stm32Tools.h"
#include "spi_low.h"
#include "CfgUser.h"

static int fd;
static struct pt pt;
static SPI_TypeDef *spi = SPI3;
static StmPIN data = { GPIOC, GPIO_Pin_11 };
static MadU8 buff[3] = { 0 };
static const char dummy[3] = { 0xA5 };

#define AD_START(p) (p)->CR1 &= ~0x0002
#define AD_END(p)   (p)->CR1 |=  0x0002

void LoHX711_Init(void)
{
    fd = open("/dev/hx711_0", _FNONBLOCK);
    if(fd < 0) {
        MAD_LOG("[HX711]Open failed!\n");
    } else {
        MAD_LOG("[HX711]Opened\n");
    }
}

char pt_LoHx711_Read(int *rc)
{
    PT_BEGIN(&pt);
    AD_START(spi);
    PT_WAIT_WHILE(&pt, StmPIN_ReadInValue(&data));
    write(fd, dummy, 3);
    PT_WAIT_UNTIL(&pt, read(fd, buff, 3) > 0);
    AD_END(spi);
    if(rc) {
        MadU32 cnt = ((MadU32)buff[0] << 16) | ((MadU32)buff[1] << 8) | (MadU32)buff[2];
        if(cnt & 0x800000) {
            *rc = 0xFF000000 | cnt;
        } else {
            *rc = cnt;
        }
    }
    PT_END(&pt);
}
