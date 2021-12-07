#include "MadDev.h"

extern MadDev_t Tty;
extern MadDev_t Ble;

MadDev_t *DevsList[] = {
    &Tty,
    &Ble,
    MAD_DEVP_END
};
