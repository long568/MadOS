#include "MadDev.h"

extern MadDev_t Ble;
extern MadDev_t I2C;

MadDev_t *DevsList[] = {
    &Ble,
    &I2C,
    MAD_DEVP_END
};
