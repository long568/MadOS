#ifndef __MAD_DRV_SDHC__H__
#define __MAD_DRV_SDHC__H__

#define SECTOR_ROLL   9
#define SECTOR_SIZE   (1 << SECTOR_ROLL)
#define CMD_RETRY_NUM 1000
#define CMD_TIME_OUT  1000 * 6
#define DAT_TIME_OUT  1000 * 30

typedef struct{
    MadU32 OCR;
} SdInfo_t;

#endif
