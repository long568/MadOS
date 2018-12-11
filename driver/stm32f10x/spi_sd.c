#include "ff.h"
#include "diskio.h"
#include "spi_sd.h"

MadU8 mSpiSd_status(mSpiSd_t *sd)
{
    return RES_OK;
}

MadU8 mSpiSd_initialize(mSpiSd_t *sd)
{
    return RES_OK;
}

MadU8 mSpiSd_read(mSpiSd_t *sd, MadU8 *buff, MadU32 sector, MadSize_t count)
{
    return RES_OK;
}

MadU8 mSpiSd_write(mSpiSd_t *sd, const MadU8 *buff, MadU32 sector, MadSize_t count)
{
    return RES_OK;
}

MadU8 mSpiSd_ioctl(mSpiSd_t *sd, MadU8 cmd,	MadVptr buff)
{
    return RES_OK;
}
