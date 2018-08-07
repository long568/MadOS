#include "MadDev.h"

extern MadDev_t Rfid0;
extern MadDev_t Lora0;

MadDev_t *DevsList[] = {
    &Rfid0,
    &Lora0,
    MNULL
};
