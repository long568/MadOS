#include "uln2803a.h"

//{0b1110, 0b1100, 0b1101, 0b1001, 0b1011, 0b0011, 0b0111, 0b0110}
const MadU8 CCW[8] = {0xE, 0xC, 0xD, 0x9, 0xB, 0x3, 0x7, 0x6};

typedef enum {
    WAIT,
    SET_IO
} DRV8825_STATE;

static MadBool ULN2803_reset(Motor *self);
static MadBool ULN2803_stop(Motor *self);
static MadBool ULN2803_step(Motor *self);
static void set_io(Motor *self);

MadBool Motor_ULN2803(struct _Motor *self)
{
    self->state = 0;
    self->dir_n = 0;
    self->dir_c = 0; // Current Index
    self->spd_n = 0;
    self->spd_c = 0;
    StmPIN_DefInitOPP(&self->io.a);
    StmPIN_DefInitOPP(&self->io.b);
    StmPIN_DefInitOPP(&self->io.c);
    StmPIN_DefInitOPP(&self->io.d);
    self->reset = ULN2803_reset;
    self->stop  = ULN2803_stop;
    self->step  = ULN2803_step;
    set_io(self);
    return MTRUE;
}

static MadBool ULN2803_reset(Motor *self)
{
    return MTRUE;
}

static MadBool ULN2803_stop(Motor *self)
{
    return MTRUE;
}

static MadBool ULN2803_step(Motor *self)
{
    if(self->spd_n == 0) 
        return MTRUE;
    
    self->spd_c++;
    if(self->spd_c >= self->spd_n) {
        self->spd_c = 0;
        if(self->dir_n) {
            if(self->dir_c == 7)
                self->dir_c = 0;
            else
                self->dir_c++;
        } else {
            if(self->dir_c == 0)
                self->dir_c = 7;
            else
                self->dir_c--;
        }
        set_io(self);
        return MTRUE;
    }
    return MFALSE;
}

static void set_io(Motor *self)
{
    MadU8   val = CCW[self->dir_c];
    MotorIO *io = &self->io;
    StmPIN_SetValue(&io->a, (val     ) & 0x01);
    StmPIN_SetValue(&io->b, (val >> 1) & 0x01);
    StmPIN_SetValue(&io->c, (val >> 2) & 0x01);
    StmPIN_SetValue(&io->d, (val >> 3) & 0x01);
}
