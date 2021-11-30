#include "GD32Tools.h"

void GPin_Init(const GPin *p, MadU32 mode, MadU32 pull_up_down, MadU8 otype, MadU32 speed)
{
    gpio_mode_set(p->port, mode, pull_up_down, p->pin);
    gpio_output_options_set(p->port, otype, speed, p->pin);
}

void GPin_SetValue(const GPin *p, MadBool v)
{
    if(v) gpio_bit_set  (p->port, p->pin);
    else  gpio_bit_reset(p->port, p->pin);
}

void GPin_Toggle(const GPin *p)
{
    gpio_bit_toggle(p->port, p->pin);
}
