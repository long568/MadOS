#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "MadOS.h"
#include "CfgUser.h"
#include "mod_Newlib.h"

MadAligned_t MadStack[MAD_OS_STACK_SIZE / MAD_MEM_ALIGN] = { 0 };
static void madStartup(MadVptr exData);

int main()
{
    rcu_periph_clock_enable(RCU_DMA);
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_GPIOC);

    madCopyVectorTab();
    madOSInit(MadStack, MAD_OS_STACK_SIZE);
    madThreadCreate(madStartup, 0, 512, 0);
    madOSRun();
	while(1);
}

static void madStartup(MadVptr exData)
{
    char *buf;

    (void)exData;
    madInitSysTick(DEF_SYS_TICK_FREQ, DEF_TICKS_PER_SEC);

    Newlib_Init();
    // MAD_LOG_INIT();

    gpio_deinit(GPIOC);
    gpio_mode_set(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, GPIO_PIN_13);
    gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_13);
    gpio_bit_set(GPIOC, GPIO_PIN_13);

    // printf("Hello World !\n");
    buf = malloc(1024);
    if(!buf) {
        while(1);
    }
    
    while(1) {
        madTimeDly(500);
        // memset(buf, 0xA5, 512);
        memset(buf, 0xA5, 1024); // Bug in DMA
        gpio_bit_toggle(GPIOC, GPIO_PIN_13);
	}
}
