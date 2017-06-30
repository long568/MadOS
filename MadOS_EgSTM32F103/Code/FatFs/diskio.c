/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2014        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"
#include "diskio.h"		/* FatFs lower layer API */
#include "stm32_eval_sdio_sd.h"

//#include "usbdisk.h"	/* Example: Header file of existing USB MSD control module */
//#include "atadrive.h"	/* Example: Header file of existing ATA harddisk control module */
//#include "sdcard.h"		/* Example: Header file of existing MMC/SDC contorl module */

/* Definitions of physical drive number for each drive */
//#define ATA		0	/* Example: Map ATA harddisk to physical drive 0 */
//#define MMC		1	/* Example: Map MMC/SD card to physical drive 1 */
//#define USB		2	/* Example: Map USB MSD to physical drive 2 */


/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	switch (pdrv) {
        case MicroSDHC0:
            if(SD_CARD_ERROR != SD_GetState()) {
                return RES_OK;
            }
            break;
            
        default: break;
	}
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	switch (pdrv) {
        case MicroSDHC0:
            SD_DeInit();
            if(SD_OK == SD_Init()) {
                return RES_OK;
            }
            break;
            
        default: break;
	}
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address in LBA */
	UINT count		/* Number of sectors to read */
)
{
    MadUint error;
	switch (pdrv) {
        case MicroSDHC0:
            if(1 == count) {
                error = SD_ReadBlock(buff, sector, 512);
            } else {
                error = SD_ReadMultiBlocks(buff, sector, 512, count);
            }
            if(SD_OK == error) {
                error = SD_WaitAnyOperation();
                if(SD_OK == error) {
                    while(SD_GetStatus() != SD_TRANSFER_OK);
                    return RES_OK;
                }
            }
            break;
            
        default: break;
	}
	return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if _USE_WRITE
DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Sector address in LBA */
	UINT count			/* Number of sectors to write */
)
{
    MadUint error;
	switch (pdrv) {
        case MicroSDHC0:
            if(1 == count) {
                error = SD_WriteBlock(buff, sector, 512);
            } else {
                error = SD_WriteMultiBlocks(buff, sector, 512, count);
            }
            if(SD_OK == error) {
                error = SD_WaitAnyOperation();
                if(SD_OK == error) {
                    while(SD_GetStatus() != SD_TRANSFER_OK);
                    return RES_OK;
                }
            }
            break;
        
        default: break;
	}
	return RES_PARERR;
}
#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

#if _USE_IOCTL
#define SD_CSD_SIZE (sizeof(SD_CSD))
#define SD_CID_SIZE (sizeof(SD_CID))
#define SDSTAT_SIZE (sizeof(SD_CardStatus))
DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	switch (pdrv) {
        case MicroSDHC0: {
            SD_CardInfo card_info;
            SD_Error card_error;
            card_error = SD_GetCardInfo(&card_info);
            if(SD_OK != card_error) return RES_PARERR;
            switch (cmd) {
                case CTRL_SYNC: 
                    return RES_OK;
                case MMC_GET_TYPE:
                    *(uint8_t*)buff = card_info.CardType;
                    return RES_OK;
                case MMC_GET_CSD: {
                    MadU32 i;
                    MadU8 *b = (MadU8*)buff;
                    MadU8 *p = (MadU8*)&card_info.SD_csd;
                    for(i=0; i<SD_CSD_SIZE; i++)
                        *b++ = *p++;
                    return RES_OK;
                }
                case MMC_GET_CID:{
                    MadU32 i;
                    MadU8 *b = (MadU8*)buff;
                    MadU8 *p = (MadU8*)&card_info.SD_cid;
                    for(i=0; i<SD_CID_SIZE; i++)
                        *b++ = *p++;
                    return RES_OK;
                }
                case MMC_GET_OCR: {
                    return RES_OK;
                }
                case MMC_GET_SDSTAT: {
                    SD_CardStatus sd_status;
                    MadU32 i;
                    MadU8 *b = (MadU8*)buff;
                    MadU8 *p = (MadU8*)&sd_status;
                    card_error = SD_GetCardStatus(&sd_status);
                    if(SD_OK != card_error) return RES_PARERR;
                    for(i=0; i<SDSTAT_SIZE; i++)
                        *b++ = *p++;
                    return RES_OK;
                }
                default: break;
            }
            break;
        }
        
        default: break;
	}
	return RES_PARERR;
}
#endif
