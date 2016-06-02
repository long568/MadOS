/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 * All rights reserved. 
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 * 
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

/* lwIP includes. */
#include "lwip/debug.h"
#include "lwip/def.h"
#include "lwip/sys.h"
#include "lwip/mem.h"
#include "lwip/memp.h"
#include "lwip/tcpip.h"

/*-----------------------------------------------------------------------------------*/
//  Creates an empty mailbox.
err_t sys_mbox_new(sys_mbox_t *mbox, int size)
{
    sys_mbox_t mb = madMsgQCreateCarefully(size, MTRUE);
    if(MNULL == mb)
        return ERR_MEM;
    *mbox = mb;
    return ERR_OK;
}

/*-----------------------------------------------------------------------------------*/
/*
  Deallocates a mailbox. If there are messages still present in the
  mailbox when the mailbox is deallocated, it is an indication of a
  programming error in lwIP and the developer should be notified.
*/

void sys_mbox_free(sys_mbox_t *mbox)
{
    MadCpsr_t cpsr;
    sys_mbox_t mb;
    struct tcpip_msg *msg;
    madEnterCritical(cpsr);
    mb = *mbox;
    while(mb->cnt) {
        madMsgCheck(mbox);
        msg = (struct tcpip_msg *)(MadCurTCB->msg);
        if(msg)
            memp_free((memp_t)(msg->type), msg);
    }
    madMsgQDelete(mbox);
    madExitCritical(cpsr);
}

/*-----------------------------------------------------------------------------------*/
//   Posts the "msg" to the mailbox.

/*-----------------------------------------------------------------------------------*/
/*
  Blocks the thread until a message arrives in the mailbox, but does
  not block the thread longer than "timeout" milliseconds (similar to
  the sys_arch_sem_wait() function). The "msg" argument is a result
  parameter that is set by the function (i.e., by doing "*msg =
  ptr"). The "msg" parameter maybe NULL to indicate that the message
  should be dropped.

  The return values are the same as for the sys_arch_sem_wait() function:
  Number of milliseconds spent waiting or SYS_ARCH_TIMEOUT if there was a
  timeout.

  Note that a function with a similar name, sys_mbox_fetch(), is
  implemented by lwIP. 
*/

u32_t sys_arch_mbox_fetch(sys_mbox_t *mbox, void **msg, u32_t timeout)
{
    MadU8 wait;
    MadU32 time, res;
    MadCpsr_t cpsr;
    time = timeout;
    wait = madMsgWait(mbox, time);
    madEnterCritical(cpsr);
    time -= MadCurTCB->timeCntRemain;
    if(MAD_ERR_OK == wait) {
        *msg = MadCurTCB->msg;
        res = time;
    } else {
        *msg = 0;
        res = SYS_MBOX_EMPTY;
    }
    madExitCritical(cpsr);
    return res;
}

u32_t sys_arch_mbox_tryfetch(sys_mbox_t *mbox, void **msg)
{
	MadCpsr_t cpsr;
    if(MAD_ERR_OK == madMsgCheck(mbox))
    {
		madEnterCritical(cpsr);
        *msg = MadCurTCB->msg;
		madExitCritical(cpsr);
        return 0;
    }
    *msg = 0;
    return SYS_MBOX_EMPTY;
}

/*-----------------------------------------------------------------------------------*/
//  Creates and returns a new semaphore. The "count" argument specifies
//  the initial state of the semaphore. TBD finish and test
err_t sys_sem_new(sys_sem_t *sem, u8_t count)
{
    sys_sem_t res = madSemCreateCarefully(count, 0xFF);
    if(MNULL == res)
        return ERR_MEM;
    *sem = res;
    return ERR_OK;
}

err_t sys_mutex_new(sys_sem_t *sem)
{
    sys_sem_t res = madSemCreateCarefully(1, 1);
    if(MNULL == res)
        return ERR_MEM;
    *sem = res;
    return ERR_OK;
}

/*-----------------------------------------------------------------------------------*/
/*
  Blocks the thread while waiting for the semaphore to be
  signaled. If the "timeout" argument is non-zero, the thread should
  only be blocked for the specified time (measured in
  milliseconds).

  If the timeout argument is non-zero, the return value is the number of
  milliseconds spent waiting for the semaphore to be signaled. If the
  semaphore wasn't signaled within the specified time, the return value is
  SYS_ARCH_TIMEOUT. If the thread didn't have to wait for the semaphore
  (i.e., it was already signaled), the function may return zero.

  Notice that lwIP implements a function with a similar name,
  sys_sem_wait(), that uses the sys_arch_sem_wait() function.
*/
u32_t sys_arch_sem_wait(sys_sem_t *sem, u32_t timeout)
{
    MadU32 time;
    MadCpsr_t cpsr;
    time = (MadU16)timeout;
    madSemWait(sem, time);
    madEnterCritical(cpsr);
    time -= MadCurTCB->timeCntRemain;
    madExitCritical(cpsr);
    return time;
}

/*-----------------------------------------------------------------------------------*/
// Signals a semaphore


/*-----------------------------------------------------------------------------------*/
// Deallocates a semaphore

/*-----------------------------------------------------------------------------------*/
// Initialize sys arch
void
sys_init(void)
{
}

/*-----------------------------------------------------------------------------------*/
/*
  Returns a pointer to the per-thread sys_timeouts structure. In lwIP,
  each thread has a list of timeouts which is represented as a linked
  list of sys_timeout structures. The sys_timeouts structure holds a
  pointer to a linked list of timeouts. This function is called by
  the lwIP timeout scheduler and must not return a NULL value. 

  In a single threaded sys_arch implementation, this function will
  simply return a pointer to a global sys_timeouts variable stored in
  the sys_arch module.
*/

/*-----------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------*/
// TBD 
/*-----------------------------------------------------------------------------------*/
/*
  Starts a new thread with priority "prio" that will begin its execution in the
  function "thread()". The "arg" argument will be passed as an argument to the
  thread() function. The id of the new thread is returned. Both the id and
  the priority are system dependent.
*/

/*
  This optional function does a "fast" critical region protection and returns
  the previous protection level. This function is only called during very short
  critical regions. An embedded system which supports ISR-based drivers might
  want to implement this function by disabling interrupts. Task-based systems
  might want to implement this by using a mutex or disabling tasking. This
  function should support recursive calls from the same task or interrupt. In
  other words, sys_arch_protect() could be called while already protected. In
  that case the return value indicates that it is already protected.

  sys_arch_protect() is only required if your port is supporting an operating
  system.
*/

/*
  This optional function does a "fast" set of critical region protection to the
  value specified by pval. See the documentation for sys_arch_protect() for
  more information. This function is only required if your port is supporting
  an operating system.
*/

