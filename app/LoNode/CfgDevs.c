#include "MadDev.h"

extern MadDev_t Tty0;
extern MadDev_t Sd0;

MadDev_t *DevsList[] = {
    &Tty0, // Tty must be the FIRST element of the DevsList.
    &Sd0,
    MNULL
};
