#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__

// enum {
//    THREAD_PRIO_SYS_RUNNING = 1,
// };

enum {
    ISR_PRIO_SYSTICK = 1,
    ISR_PRIO_ARCH_MEM,
    ISR_PRIO_PENDSV  = 15
};

#define MAD_OS_STACK_SIZE (56 * 1024)

#endif
