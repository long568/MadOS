#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__

#define TEST_STR "long, 哈哈, we will we will fuck you !\r\n"

enum {
    THREAD_PRIO_SYS_RUNNING     = 1,
    THREAD_PRIO_MAD_ARM         = 12,
    THREAD_PRIO_DRIVER_ETH      = 20,
    THREAD_PRIO_TEST_SPI_FLASH  = 50,
};

enum {
    ISR_PRIO_SYSTICK    = 1,
    ISR_PRIO_TIM6,
    ISR_PRIO_ARCH_MEM,
	ISR_PRIO_IP101A,
    ISR_PRIO_W25Q32,
    ISR_PRIO_TTY_USART,
    ISR_PRIO_PENDSV     = 15
};

#define SYS_RUNNING_INTERVAL_MSECS (500)
#define MAD_OS_STACK_SIZE          (56 * 1024)

#endif
