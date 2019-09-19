/****************************************
 * Interface  : SPI
 * File System: Fatfs R0.13c
 ****************************************/
#include <stdarg.h>
#include <stdlib.h>
#include "MadDev.h"
#include "spi_low.h"
#include "MadDrvSdhc.h"
#include "mstd_crc.h"
#include "../library/FatFs/diskio.h"

#define CMD0    0
#define CMD1    1
#define CMD8    8
#define CMD12  12
#define CMD16  16
#define CMD17  17
#define CMD18  18
#define CMD24  24
#define CMD25  25
#define CMD55  55
#define CMD58  58
#define ACMD41 41

static void mSpiSd_SetCmd(MadU8 *buf, MadU8 cmd, MadU32 arg)
{
    int i;
    MadU8 *parg = (MadU8*)(&arg);
    buf[0] = 0x40 | cmd;
    for(i=1; i<5; i++) {
        buf[i] = parg[4-i];
    }
    switch (cmd)
    {
        case CMD0:
        case CMD8:
            buf[5] = (CRC7(buf, 5) << 1) | 0x01;
            break;
        default:
            buf[5] = 0xFF;
            break;
    }
}

static MadU8 mSpiSd_BootCmd(mSpi_t *spi, MadU8 *buf, MadU32 *rep)
{
    int i;
    MadU8 *prep;
    MadU8 r, res;
    res = mSpi_INVALID_DATA;
    prep = (MadU8*)rep;
    if(MTRUE == mSpiWriteBytes(spi, buf, 6, CMD_TIME_OUT)) {
        r = 0xFF;
        for(i=0; i<OPT_RETRY_NUM; i++) {
            mSpiRead8Bit(spi, &r);
            if(r & 0x80) {
                continue;
            }
            res = r;
            if(rep) {
                MadU8 tmp[4] = { 0 };
                if(MTRUE == mSpiReadBytes(spi, tmp, 4, CMD_TIME_OUT)) {
                    prep[3] = tmp[0];
                    prep[2] = tmp[1];
                    prep[1] = tmp[2];
                    prep[0] = tmp[3];
                }
            }
            break;
        }
    }
    return res;
}

inline
static MadU8 mSpiSd_SendCmd(mSpi_t *spi, MadU8 cmd, MadU32 arg, MadU32 *rep)
{
    MadU8 res;
    MadU8 buf[6];
    mSpiSd_SetCmd(buf, cmd, arg);
    mSpi_NSS_ENABLE(spi);
    res = mSpiSd_BootCmd(spi, buf, rep);
    mSpi_NSS_DISABLE(spi);
    mSpiSend8BitInvalid(spi);
    return res;
}

inline
static int mSpiSd_WriteCmd(mSpi_t *spi, MadU8 cmd, MadU32 arg, const MadU8 r1, MadU32 *rep)
{
    if(r1 == mSpiSd_SendCmd(spi, cmd, arg, rep)) {
        return 1;
    } else {
        return -1;
    }
}

inline
static int mSpiSd_Reset(mSpi_t *spi)
{
    MAD_LOG("[SD] Reset\n");
    madTimeDly(100);
    mSpiMulEmpty(spi, 10, DAT_TIME_OUT);
    return mSpiSd_WriteCmd(spi, CMD0, 0, 0x01, 0);
}

inline
static int mSpiSd_OCR(mSpi_t *spi, MadU32 *rep)
{
    return mSpiSd_WriteCmd(spi, CMD58, 0, 0x00, rep);
}

inline
static int mSpiSd_Init(mSpi_t *spi)
{
    int cnt, res;
    MadU32 rep;
    rep = 0;
    res = mSpiSd_WriteCmd(spi, CMD8, 0x000001AA, 0x01, &rep);
    if(0 < res) {
        MAD_LOG("[SD] SDC v2+ 0x%08X\n", rep);
        rep &= 0x00000FFF;
        if(rep == 0x1AA){
            for(cnt=0; cnt<OPT_RETRY_NUM; cnt++) {
                if((0 < mSpiSd_WriteCmd(spi, CMD55,  0x00000000, 0x01, 0)) &&
                   (0 < mSpiSd_WriteCmd(spi, ACMD41, 0x40000000, 0x00, 0))) {
                    res = 1;
                    break;
                }
            }
        }
    } else {
        MAD_LOG("[SD] SDC v1\n");
        if(((0 < mSpiSd_WriteCmd(spi, CMD55,  0x00000000, 0x01, 0)) &&
            (0 < mSpiSd_WriteCmd(spi, ACMD41, 0x40000000, 0x00, 0)))||
            (0 < mSpiSd_WriteCmd(spi, CMD1,   0x00000000, 0x00, 0))) {
            res = 1;
        }
    }
    return res;
}

inline
static int mSpiSd_SetBlkSize(mSpi_t *spi)
{
    MAD_LOG("[SD] SetBlkSize\n");
    return mSpiSd_WriteCmd(spi, CMD16, SECTOR_SIZE, 0x00, 0);
}

inline
static int mSpiSd_WaitIdle(mSpi_t *spi)
{
    int cnt;
    MadU8 tmp = 0;
    for(cnt=0; cnt<OPT_RETRY_NUM; cnt++) {
        mSpiMulRead(spi, &tmp, 8, DAT_TIME_OUT);
        if(tmp == 0xFF) return 1;
    }
    MAD_LOG("[SD] WaitIdle ERROR\n");
    return -1;
}

static int mSpiSd_ReadOneSector(mSpi_t *spi, MadU8 *buff)
{
    int cnt, res;
    MadU8 tmp;
    res = -1;
    tmp = mSpi_INVALID_DATA;
    for(cnt=0; cnt<OPT_RETRY_NUM; cnt++) {
        mSpiRead8Bit(spi, &tmp);
        if(tmp == 0xFE) break;
    }
    if(tmp == 0xFE) {
        if(MTRUE == mSpiReadBytes(spi, buff, SECTOR_SIZE, DAT_TIME_OUT)) {
            mSpiMulEmpty(spi, 2, DAT_TIME_OUT);
            res = 1;
        } else {
            MAD_LOG("[SD] ReadOneSector->ReadBytes ERROR\n");
        }
    } else {
        MAD_LOG("[SD] ReadOneSector->Read8Bit ERROR[%02X][%d]\n", tmp, cnt);
    }
    return res;
}

static int mSpiSd_WriteOneSector(mSpi_t *spi, const MadU8 *buff, MadU8 head)
{
    int cnt, res;
    MadU8 tmp;
    res = -1;
    if(MTRUE == mSpiSend8Bit(spi, head)) {
        if(MTRUE == mSpiWriteBytes(spi, buff, SECTOR_SIZE, DAT_TIME_OUT)) {
            mSpiMulEmpty(spi, 2, DAT_TIME_OUT);
            tmp = mSpi_INVALID_DATA;
            for(cnt=0; cnt<OPT_RETRY_NUM; cnt++) {
                mSpiRead8Bit(spi, &tmp);
                if(tmp != 0xFF) break;
            }
            switch (tmp & 0x1F)
            {
                case 0x05:
                    res = 1;
                    break;
                case 0x0B:
                case 0x0D:
                default:
                    MAD_LOG("[SD] WriteOneSector->Read8Bit ERROR[%d]\n", tmp);
                    break;
            }
            mSpiSd_WaitIdle(spi);
        } else {
            MAD_LOG("[SD] WriteOneSector->WriteBytes ERROR\n");
        }
    } else {
        MAD_LOG("[SD] WriteOneSector->Send8Bit ERROR\n");
    }
    return res;
}

static int mSpiSd_read(mSpi_t *spi, MadU8 *data, MadU32 sector, MadU32 count, SdType_t type)
{
    int i, res;
    MadU8 buf[6];
    MadU32 addr;
    res = -1;
    if(count == 0) return res;
    if(type == SdType_SC) {
        addr = sector << SECTOR_ROLL;
    } else {
        addr = sector;
    }
    mSpi_NSS_ENABLE(spi);
    if(count == 1) {
        mSpiSd_SetCmd(buf, CMD17, addr);
        if(0x00 == mSpiSd_BootCmd(spi, buf, 0)) {
            res = mSpiSd_ReadOneSector(spi, data);
        } else {
            MAD_LOG("[SD] read(1)->BootCmd ERROR\n");
        }
    } else {
        mSpiSd_SetCmd(buf, CMD18, addr);
        if(0x00 == mSpiSd_BootCmd(spi, buf, 0)) {
            for(i=0; i<count; i++) {
                res   = mSpiSd_ReadOneSector(spi, data);
                data += SECTOR_SIZE;
                if(res < 0) break;
            }
            mSpiSd_SetCmd(buf, CMD12, 0);
            mSpiSd_BootCmd(spi, buf, 0);
            mSpiSd_WaitIdle(spi);
        } else {
            MAD_LOG("[SD] read(n)->BootCmd ERROR\n");
        }
    }
    mSpi_NSS_DISABLE(spi);
    return res;
}

static int mSpiSd_write(mSpi_t *spi, const MadU8 *data, MadU32 sector, MadU32 count, SdType_t type)
{
    int i, res;
    MadU8 buf[6];
    MadU32 addr;
    res = -1;
    if(count == 0) return res;
    if(type == SdType_SC) {
        addr = sector << SECTOR_ROLL;
    } else {
        addr = sector;
    }
    mSpi_NSS_ENABLE(spi);
    if(count == 1) {
        mSpiSd_SetCmd(buf, CMD24, addr);
        if(0x00 == mSpiSd_BootCmd(spi, buf, 0)) {
            mSpiSend8BitInvalid(spi);
            res = mSpiSd_WriteOneSector(spi, data, 0xFE);
        } else {
            MAD_LOG("[SD] write(1)->BootCmd ERROR\n");
        }
    } else {
        mSpiSd_SetCmd(buf, CMD25, addr);
        if(0x00 == mSpiSd_BootCmd(spi, buf, 0)) {
            for(i=0; i<count; i++) {
                res   = mSpiSd_WriteOneSector(spi, data, 0xFC);
                data += SECTOR_SIZE;
                if(res < 0) break;
            }
            mSpiSend8Bit(spi, 0xFD);
            mSpiSd_WaitIdle(spi);
        } else {
            MAD_LOG("[SD] write(n)->BootCmd ERROR\n");
        }
    }
    mSpi_NSS_DISABLE(spi);
    return res;
}

static int Drv_open  (const char *, int, va_list);
static int Drv_close (int fd);
static int Drv_ioctl (int fd, int request, va_list args);


const MadDrv_t MadDrvSdhc = {
    Drv_open,
    0,
    0,
    0,
    0,
    Drv_close,
    0,
    0,
    Drv_ioctl
};

static int Drv_open(const char * file, int flag, va_list args)
{
    int      i;
    SdInfo_t *sd_info;
    int      fd   = (int)file;
    MadDev_t *dev = DevsList[fd];
    mSpi_t   *spi = (mSpi_t *)(dev->dev);
    mSpi_InitData_t *initData = (mSpi_InitData_t*)(dev->args);

    (void)args;

    dev->flag     = flag;
    dev->ptr      = malloc(sizeof(SdInfo_t));
    dev->txBuff   = 0;
    dev->rxBuff   = 0;
    dev->txLocker = 0;
    dev->rxLocker = 0;
    if(0 == dev->ptr) return -1;
    sd_info = (SdInfo_t*)(dev->ptr);
    if(MFALSE == mSpiInit(spi, initData)) {
        free(dev->ptr);
        return -1;
    }

    i = 1;
    do {
        madTimeDly(1000);
        do {
            MAD_LOG("[SD] Startup ... [%d]\n", i);
            mSpiSetClkPrescaler(spi, SPI_BaudRatePrescaler_256);
            if(0 > mSpiSd_Reset(spi)) break;
            if(0 > mSpiSd_Init(spi)) break;
            mSpiSetClkPrescaler(spi, SPI_BaudRatePrescaler_4);
            madTimeDly(10);
            if(0 > mSpiSd_OCR(spi, &sd_info->OCR)) break;
            MAD_LOG("[SD] OCR = 0x%08X\n", sd_info->OCR);
            if(sd_info->OCR & 0x40000000) {
                sd_info->type = SdType_HC;
            } else {
                sd_info->type = SdType_SC;
                if(0 > mSpiSd_SetBlkSize(spi)) break;
            }
            MAD_LOG("[SD] Ready (%s)\n", (sd_info->type == SdType_SC) ? "SDSC" : "SDHC");
            return 1;
        } while(0);
    } while(i++ < 10);

    free(dev->ptr);
    mSpiDeInit(spi);
    MAD_LOG("[SD] Error\n");
    return -1;
}

static int Drv_close(int fd)
{
    MadDev_t *dev = DevsList[fd];
    mSpi_t   *spi = (mSpi_t *)(dev->dev);
    free(dev->ptr);
    mSpiDeInit(spi);
    MAD_LOG("[SD]...Closed\n");
    return 0;
}

static int Drv_ioctl(int fd, int request, va_list args)
{
    int res;
    MadDev_t *dev     = DevsList[fd];
    mSpi_t   *spi     = (mSpi_t*)(dev->dev);
    SdInfo_t *sd_info = (SdInfo_t*)(dev->ptr);

    res = -1;
    switch (request)
    {
        case F_DISK_STATUS: {
#if 0
            res = mSpiSd_OCR(spi, &sd_info->OCR);
            if(res < 0) 
                MAD_LOG("[SD] F_DISK_STATUS(%d) Error\n", res);
#else
            res = 1;
#endif
            break;
        }

        case F_DISK_READ: {
            MadU8* buff;
            MadU32 sector;
            MadU32 count;
            buff   = va_arg(args, MadU8*);
            sector = va_arg(args, MadU32);
            count  = va_arg(args, MadU32);
            res = mSpiSd_read(spi, buff, sector, count, sd_info->type);
            if(res < 0) 
                MAD_LOG("[SD] F_DISK_READ(%d, %d, %d) Error\n", res, sector, count);
            break;
        }

        case F_DISK_WRITE: {
            const MadU8* buff;
            MadU32 sector;
            MadU32 count;
            buff   = va_arg(args, MadU8*);
            sector = va_arg(args, MadU32);
            count  = va_arg(args, MadU32);
            res = mSpiSd_write(spi, buff, sector, count, sd_info->type);
            if(res < 0) 
                MAD_LOG("[SD] F_DISK_WRITE(%d, %d, %d) Error\n", res, sector, count);
            break;
        }

        case CTRL_SYNC: {
            res = 1;
            break;
        }

        default:
            MAD_LOG("[SD] Unknown CMD (%d)\n", request);
            break;
    }
    return res;
}
