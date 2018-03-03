#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__

enum {
    THREAD_PRIO_SYS_RUNNING     = 1,
    THREAD_PRIO_TEST_MEM_0      = 13,
    THREAD_PRIO_TEST_MEM_1,
    THREAD_PRIO_TEST_MEM,
};

enum {
    ISR_PRIO_SYSTICK    = 1,
    ISR_PRIO_ARCH_MEM,
    ISR_PRIO_PENDSV     = 15
};

#define SYS_RUNNING_INTERVAL_MSECS (500)
#define MAD_OS_STACK_SIZE          (60 * 1024)

#endif
