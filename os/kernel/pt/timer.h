#ifndef __MAD_LIB_TIMER__H__
#define __MAD_LIB_TIMER__H__

#include "MadOS.h"

typedef MadTime_t clock_time_t;

struct _tmr_t;
struct _clkr_t;

typedef struct _tmr_t{
    struct _clkr_t  *c;
    struct _tmr_t   *next;
    clock_time_t    ofl;
    clock_time_t    com;
    clock_time_t    tout;
    clock_time_t    inval;
    MadBool         expired;
} tmr;

typedef struct _clkr_t{
    clock_time_t   cur;
    struct _tmr_t  *tlist;
} clkr;

void clkr_init(clkr *c);
void clkr_dt(clkr *c, clock_time_t dt);

void tmr_init(tmr *t);
void tmr_add(tmr *t, clkr *clkr, clock_time_t interval);
void tmr_set(tmr *t, clock_time_t interval); // Interval should never be 0.
void tmr_restart(tmr *t);
void tmr_remove(tmr *t);
int  tmr_expired(tmr *t);

#endif /* __TIMER_H__ */
