#include "MadDev.h"

extern MadDev_t Tty;
extern MadDev_t Sd0;

MadDev_t *DevsList[] = {
    &Tty, // Tty must be the FIRST element of the DevsList.
    &Sd0,
    MNULL
};
