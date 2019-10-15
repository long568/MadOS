#include "MadDev.h"

extern MadDev_t Tty;
extern MadDev_t Tty1;
extern MadDev_t Sd0;

MadDev_t *DevsList[] = {
    &Tty,
    &Tty1,
    &Sd0,
    MNULL
};
