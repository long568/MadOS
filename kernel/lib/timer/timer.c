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
