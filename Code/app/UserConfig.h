#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__

#define TEST_STR "long, 哈哈, we will we will fuck you !"

enum {
    THREAD_PRIO_SYS_RUNNING     = 1,
	THREAD_PRIO_LWIP_ISR        = 16,
    THREAD_PRIO_LWIP_TCPIP,
	THREAD_PRIO_TCPSOCKET_TEST
};

enum {
    ISR_PRIO_SYSTICK    = 1,
    ISR_PRIO_ARCH_MEM,
	ISR_PRIO_ENC28J60,
    ISR_PRIO_MicroSD,
    ISR_PRIO_PENDSV     = 15
};

#define SYS_RUNNING_INTERVAL_MSECS 500

#endif
