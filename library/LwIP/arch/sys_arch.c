/*
 * Copyright (c) 2017 Simon Goldschmidt
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
 * Author: Simon Goldschmidt
 *
 */


#include <lwip/opt.h>
#include <lwip/arch.h>
#if !NO_SYS
#include "arch/sys_arch.h"
#endif
#include <lwip/stats.h>
#include <lwip/debug.h>
#include <lwip/sys.h>

#include <string.h>

// u32_t lwip_sys_now;

// u32_t
// sys_jiffies(void)
// {
//     return lwip_sys_now;
// }

inline u32_t
sys_now(void)
{
    return madTimeNow();
}

void
sys_init(void)
{
}

#if !NO_SYS

err_t
sys_sem_new(sys_sem_t *sem, u8_t count)
{
    err_t rc = ERR_OK;
    *sem = madSemCreate(count);
    if(*sem == NULL) {
        rc = ERR_MEM;
    }
    return rc;
}

void
sys_sem_free(sys_sem_t *sem)
{
    madSemDelete(sem);
}

void
sys_sem_set_invalid(sys_sem_t *sem)
{
    *sem = NULL;
}

u32_t
sys_arch_sem_wait(sys_sem_t *sem, u32_t timeout)
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

void
sys_sem_signal(sys_sem_t *sem)
{
    madSemRelease(sem);
}

err_t
sys_mutex_new(sys_mutex_t *mutex)
{
    err_t rc = ERR_OK;
    *mutex = madMutexCreate();
    if(*mutex == NULL) {
        rc = ERR_MEM;
    }
    return rc;
}

void
sys_mutex_free(sys_mutex_t *mutex)
{
    madMutexDelete(mutex);
}

void
sys_mutex_set_invalid(sys_mutex_t *mutex)
{
    *mutex = NULL;
}

void
sys_mutex_lock(sys_mutex_t *mutex)
{
    madMutexWait(mutex, 0);
}

void
sys_mutex_unlock(sys_mutex_t *mutex)
{
    madMutexRelease(mutex);
}

sys_thread_t
sys_thread_new(const char *name, lwip_thread_fn function, void *arg, int stacksize, int prio)
{
    (void) name;
    return madThreadCreate(function, arg, stacksize, prio);
}

err_t
sys_mbox_new(sys_mbox_t *mbox, int size)
{
    err_t rc = ERR_OK;
    *mbox = madMsgQCreateCarefully(size, MTRUE);
    if(*mbox == NULL) {
        rc = ERR_MEM;
    }
    return rc;
}

void
sys_mbox_free(sys_mbox_t *mbox)
{
    madMsgQDelete(mbox);
}

void
sys_mbox_set_invalid(sys_mbox_t *mbox)
{
    *mbox = NULL;
}

void
sys_mbox_post(sys_mbox_t *q, void *msg)
{
    madMsgSendBlock(q, msg, 0);
}

err_t
sys_mbox_trypost(sys_mbox_t *q, void *msg)
{
    err_t rc = ERR_OK;
    if(MAD_ERR_OK != madMsgSend(q, msg)) {
        rc = ERR_MEM;
    }
    return rc;
}

err_t
sys_mbox_trypost_fromisr(sys_mbox_t *q, void *msg)
{
    return sys_mbox_trypost(q, msg);
}

u32_t
sys_arch_mbox_fetch(sys_mbox_t *q, void **msg, u32_t timeout)
{
    MadU8     wait;
    MadU32    time;
    MadU32    res;
    MadVptr   massage;
    MadCpsr_t cpsr;
    
    time = timeout;
    wait = madMsgWait(q, &massage, time);
    madEnterCritical(cpsr);
    time -= MadCurTCB->timeCntRemain;
    madExitCritical(cpsr);
    if(MAD_ERR_OK == wait) {
        if(msg) *msg = massage;
        res = time;
    } else {
        if(msg) *msg = 0;
        res = SYS_MBOX_EMPTY;
    }
    return res;
}

u32_t
sys_arch_mbox_tryfetch(sys_mbox_t *q, void **msg)
{
    MadU32  res;
    MadVptr massage;
    if(MAD_ERR_OK == madMsgCheck(q, &massage)) {
        if(msg) *msg = massage;
        res = 0;
    } else {
        if(msg) *msg = 0;
        res = SYS_MBOX_EMPTY;
    }
    return res;
}

#if LWIP_NETCONN_SEM_PER_THREAD
#error LWIP_NETCONN_SEM_PER_THREAD==1 not supported
#endif /* LWIP_NETCONN_SEM_PER_THREAD */

#endif /* !NO_SYS */
