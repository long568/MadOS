/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2016        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"			/* Obtains integer types */
#include "diskio.h"		/* Declarations of disk functions */
#include "spi_sd.h"

/* Definitions of physical drive number for each drive */
#define DEV_RAM		0	/* Example: Map Ramdisk to physical drive 0 */
#define DEV_SDC		1	/* Example: Map MMC/SD card to physical drive 1 */
#define DEV_USB		2	/* Example: Map USB MSD to physical drive 2 */

static mSpiSd_t sd0;

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	DRESULT res;

	switch (pdrv) {
		case DEV_RAM:
			res = STA_NODISK;
			break;

		case DEV_SDC:
			res = mSpiSd_status(&sd0);
			break;

		case DEV_USB:
			res = STA_NODISK;
			break;

		default:
			res = STA_NODISK;
			break;
	}

	return res;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	DRESULT res;
	
	switch (pdrv) {
		case DEV_RAM:
			res = STA_NODISK;
			break;

		case DEV_SDC:
			res = mSpiSd_initialize(&sd0);
			break;

		case DEV_USB:
			res = STA_NODISK;
			break;

		default:
			res = STA_NODISK;
			break;
	}

	return res;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	DRESULT res;
	
	switch (pdrv) {
		case DEV_RAM:
			res = STA_NODISK;
			break;

		case DEV_SDC:
			res = mSpiSd_read(&sd0, buff, sector, count);
			break;

		case DEV_USB:
			res = STA_NODISK;
			break;

		default:
			res = STA_NODISK;
			break;
	}

	return res;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	DRESULT res;
	
	switch (pdrv) {
		case DEV_RAM:
			res = STA_NODISK;
			break;

		case DEV_SDC:
			res = mSpiSd_write(&sd0, buff, sector, count);
			break;

		case DEV_USB:
			res = STA_NODISK;
			break;

		default:
			res = STA_NODISK;
			break;
	}

	return res;
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res;
	
	switch (pdrv) {
		case DEV_RAM:
			res = STA_NODISK;
			break;

		case DEV_SDC:
			res = mSpiSd_ioctl(&sd0, cmd, buff);
			break;

		case DEV_USB:
			res = STA_NODISK;
			break;

		default:
			res = STA_NODISK;
			break;
	}

	return res;
}
