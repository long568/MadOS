#include "MadDev.h"

extern MadDev_t Tty;

MadDev_t *DevsList[] = {
    &Tty, // Tty must be the FIRST element of the DevsList.
    MNULL
};
