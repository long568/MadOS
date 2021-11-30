#include "timer.h"

void clkr_init(clkr *c)
{
    c->cur   = 0;
    c->tlist = 0;
}

void clkr_dt(clkr *c, clock_time_t dt)
{
    MadU8 rev;
    tmr *p_tmr;
    clock_time_t old;
    
    if(0 == dt) return;
    
    old = c->cur;
    c->cur += dt;
    if(c->cur > old) rev = MFALSE;
    else                rev = MTRUE;
    
    p_tmr = c->tlist;
    while(p_tmr) {
        do {
            if(p_tmr->expired) 
                break;
            if(rev) {
                if(p_tmr->ofl) p_tmr->ofl = 0;
                else goto LABEL_tmr_expired;
            }
            if(p_tmr->ofl) break;
            if(c->cur < p_tmr->tout) break;
LABEL_tmr_expired:
            p_tmr->expired = 1;
        } while(0);
        p_tmr = p_tmr->next;
    }
}

void tmr_init(tmr *t)
{
    t->c       = 0;
    t->next    = 0;
    t->ofl     = 0;
    t->tout    = 0;
    t->inval   = 0;
    t->expired = 0;
}

void tmr_add(tmr *t, clkr *c)
{
    if(c) {
        tmr_remove(t);
        t->c       = c;
        t->expired = 1;
        t->next    = c->tlist;
        c->tlist   = t;
    }
}

void tmr_set(tmr *t, clock_time_t interval)
{
    clkr *c = t->c;
    if(c) {
        t->inval = interval;
        tmr_restart(t);
    }
}

void tmr_restart(tmr *t)
{
    clkr *c = t->c;
    if(c) {
        t->tout    = c->cur + t->inval;
        t->expired = 0;
        if(t->tout > c->cur) t->ofl = 0;
        else                 t->ofl = 1;
    }
}

void tmr_remove(tmr *t)
{
    tmr *pre_tmr;
    tmr *cur_tmr;
    clkr *c = t->c;
    if(c) {
        pre_tmr = 0;
        cur_tmr = c->tlist;
        while(cur_tmr) {
            if(t == cur_tmr) {
                if(pre_tmr) pre_tmr->next = cur_tmr->next;
                else          c->tlist    = cur_tmr->next;
                tmr_init(t);
                break;
            }
            pre_tmr = cur_tmr;
            cur_tmr = cur_tmr->next;
        }
    }
}

int tmr_expired(tmr *t)
{
    clkr *c = t->c;
    if(c && t->expired) {
        tmr_restart(t);
        return 1;
    } else {
        return 0;
    }
}
