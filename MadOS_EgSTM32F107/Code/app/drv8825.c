#include "drv8825.h"

typedef enum {
    SET_HIGH,
    SET_LOW,
    WAIT
} DRV8825_STATE;

static MadBool DRV8825_reset(Motor *self);
static MadBool DRV8825_stop(Motor *self);
static MadBool DRV8825_step(Motor *self);

MadBool Motor_DRV8825(Motor *self)
{
    self->state = SET_HIGH;
    self->dir_n = 0;
    self->dir_c = 0;
    self->spd_n = 0;
    self->spd_c = 0;
    StmPIN_DefInitOPP(&self->io.a); // Dir
    StmPIN_DefInitOPP(&self->io.b); // Step
    self->reset = DRV8825_reset;
    self->stop  = DRV8825_stop;
    self->step  = DRV8825_step;
    StmPIN_SetLow(&self->io.a);
    StmPIN_SetLow(&self->io.b);
    return MTRUE;
}

static MadBool DRV8825_reset(Motor *self)
{
    return MTRUE;
}

static MadBool DRV8825_stop(Motor *self)
{
    return MTRUE;
}

static MadBool DRV8825_step(Motor *self)
{
    MadBool res = MFALSE;
    
    if(self->spd_n == 0) 
        return MTRUE;
    
    switch (self->state) {
        case SET_HIGH:
            if(self->dir_n != self->dir_c) {
                self->dir_c = self->dir_n;
                StmPIN_SetValue(&self->io.a, self->dir_c);
            } else {
                StmPIN_SetHigh(&self->io.b);
                self->state++;
            }
            break;
        case SET_LOW:
            if(self->spd_c == self->spd_n) {
                self->spd_c = 0;
                StmPIN_SetLow(&self->io.b);
                if(self->spd_n == 1) {
                    goto DRV8825_step_end;
                } else {
                    self->state++;
                }
            }
            break;
        case WAIT:
            if(self->spd_c == self->spd_n - 1) {
                self->spd_c = 0;
DRV8825_step_end:
                self->state = SET_HIGH;
                return MTRUE;
            }
            break;
        default:
            break;
    }
    
    self->spd_c++;
    return res;
}
