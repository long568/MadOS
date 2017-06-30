/**
 * \addtogroup timer
 * @{
 */

/**
 * \file
 * Timer library implementation.
 * \author
 * Adam Dunkels <adam@sics.se>
 */

/*
 * Copyright (c) 2004, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the uIP TCP/IP stack
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 * $Id: timer.c,v 1.2 2006/06/12 08:00:30 adam Exp $
 */

#include "clock.h"
#include "timer.h"

void clocker_init(clocker *clkr)
{
    clkr->cur   = 0;
    clkr->tlist = 0;
}

void clocker_dt(clocker *clkr, clock_time_t dt)
{
    MadU8 rev;
    timer *p_timer;
    clock_time_t old;
    
    if(0 == dt) return;
    
    old = clkr->cur;
    clkr->cur += dt;
    if(clkr->cur > old) rev = MFALSE;
    else                rev = MTRUE;
    
    p_timer = clkr->tlist;
    while(p_timer) {
        do {
            if(p_timer->expired) 
                break;
            if(rev) {
                if(p_timer->ofl) p_timer->ofl = 0;
                else goto LABEL_timer_expired;
            }
            if(p_timer->ofl) break;
            if(clkr->cur < p_timer->tout) break;
LABEL_timer_expired:
            p_timer->expired = 1;
        } while(0);
        p_timer = p_timer->next;
    }
}

void timer_init(timer *t)
{
    t->clkr    = 0;
    t->next    = 0;
    t->ofl     = 0;
    t->tout    = 0;
    t->inval   = 0;
    t->expired = 0;
}

void timer_add(timer *t, clocker *clkr)
{
    if(clkr) {
        timer_remove(t);
        t->clkr = clkr;
        t->expired = 1;
        t->next = clkr->tlist;
        clkr->tlist = t;
    }
}

void timer_set(timer *t, clock_time_t interval)
{
    clocker *clkr = t->clkr;
    if(clkr) {
        t->inval = interval;
        timer_restart(t);
    }
}

void timer_restart(timer *t)
{
    clocker *clkr = t->clkr;
    if(clkr) {
        t->tout    = clkr->cur + t->inval;
        t->expired = 0;
        if(t->tout > clkr->cur) t->ofl = 0;
        else                    t->ofl = 1;
    }
}

void timer_remove(timer *t)
{
    timer *pre_timer;
    timer *cur_timer;
    clocker *clkr = t->clkr;
    if(clkr) {
        pre_timer = 0;
        cur_timer = clkr->tlist;
        while(cur_timer) {
            if(t == cur_timer) {
                if(pre_timer) pre_timer->next = cur_timer->next;
                else          clkr->tlist     = cur_timer->next;
                timer_init(t);
                break;
            }
            pre_timer = cur_timer;
            cur_timer = cur_timer->next;
        }
    }
}

int timer_expired(timer *t)
{
    clocker *clkr = t->clkr;
    if(clkr && t->expired) {
        timer_restart(t);
        return 1;
    } else {
        return 0;
    }
}

/*---------------------------------------------------------------------------*/
/**
 * Set a timer.
 *
 * This function is used to set a timer for a time sometime in the
 * future. The function timer_expired() will evaluate to true after
 * the timer has expired.
 *
 * \param t A pointer to the timer
 * \param interval The interval before the timer expires.
 *
 */
//void
//timer_set(struct timer *t, clock_time_t interval)
//{
//  t->interval = interval;
//  t->start = clock_time();
//}
/*---------------------------------------------------------------------------*/
/**
 * Reset the timer with the same interval.
 *
 * This function resets the timer with the same interval that was
 * given to the timer_set() function. The start point of the interval
 * is the exact time that the timer last expired. Therefore, this
 * function will cause the timer to be stable over time, unlike the
 * timer_rester() function.
 *
 * \param t A pointer to the timer.
 *
 * \sa timer_restart()
 */
//void
//timer_reset(struct timer *t)
//{
//  t->start += t->interval;
//}
/*---------------------------------------------------------------------------*/
/**
 * Restart the timer from the current point in time
 *
 * This function restarts a timer with the same interval that was
 * given to the timer_set() function. The timer will start at the
 * current time.
 *
 * \note A periodic timer will drift if this function is used to reset
 * it. For preioric timers, use the timer_reset() function instead.
 *
 * \param t A pointer to the timer.
 *
 * \sa timer_reset()
 */
//void
//timer_restart(struct timer *t)
//{
//  t->start = clock_time();
//}
/*---------------------------------------------------------------------------*/
/**
 * Check if a timer has expired.
 *
 * This function tests if a timer has expired and returns true or
 * false depending on its status.
 *
 * \param t A pointer to the timer
 *
 * \return Non-zero if the timer has expired, zero otherwise.
 *
 */
//int
//timer_expired(struct timer *t)
//{
//  return (clock_time_t)(clock_time() - t->start) >= (clock_time_t)t->interval;
//}
/*---------------------------------------------------------------------------*/

/** @} */
