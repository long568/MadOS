#ifndef __SPI_SD__H__
#define __SPI_SD__H__

#include "spi_low.h"

typedef struct __mSpiSd_t {
    mSpi_t port;
} mSpiSd_t;

extern MadU8 mSpiSd_status    (mSpiSd_t *sd);
extern MadU8 mSpiSd_initialize(mSpiSd_t *sd);
extern MadU8 mSpiSd_read      (mSpiSd_t *sd,       MadU8 *buff, MadU32 sector, MadSize_t count);
extern MadU8 mSpiSd_write     (mSpiSd_t *sd, const MadU8 *buff, MadU32 sector, MadSize_t count);
extern MadU8 mSpiSd_ioctl     (mSpiSd_t *sd,       MadU8   cmd, MadVptr  buff);

#endif
