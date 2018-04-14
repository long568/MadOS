#ifndef __MAD_LIB_TIMER__H__
#define __MAD_LIB_TIMER__H__

#include "MadOS.h"

typedef MadTim_t clock_time_t;

struct _timer_t;
struct _clocker_t;

typedef struct _timer_t{
    struct _clocker_t *clkr;
    struct _timer_t   *next;
    clock_time_t      ofl;
    clock_time_t      tout;
    clock_time_t      inval;
    MadBool           expired;
} timer;

typedef struct _clocker_t{
    clock_time_t    cur;
    struct _timer_t *tlist;
} clocker;

void clocker_init(clocker *clkr);
void clocker_dt(clocker *clkr, clock_time_t dt);

void timer_init(timer *t);
void timer_add(timer *t, clocker *clkr);
void timer_set(timer *t, clock_time_t interval); // Interval should never be 0.
void timer_restart(timer *t);
void timer_remove(timer *t);
int  timer_expired(timer *t);

#endif /* __TIMER_H__ */
