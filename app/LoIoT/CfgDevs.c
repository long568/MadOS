#include "MadDev.h"

extern MadDev_t Tty0;
extern MadDev_t Lora0;

MadDev_t *DevsList[] = {
    &Tty0, // Tty must be the FIRST element of the DevsList.
    &Lora0,
    MNULL
};