#ifndef __MAD_DRV_SDHC__H__
#define __MAD_DRV_SDHC__H__

#define SECTOR_ROLL       9
#define SECTOR_SIZE       (1 << SECTOR_ROLL)
#define STARTUP_RETRY_NUM 10
#define CMD_RETRY_NUM     1000
#define IDLE_RETRY_NUM    10000
#define CMD_TIME_OUT      1000 * 6
#define DAT_TIME_OUT      1000 * 30

typedef enum {
    SdType_SC,
    SdType_HC,
    SdType_XC,
} SdType_t;

typedef struct{
    MadU32   OCR;
    SdType_t type;
} SdInfo_t;

#endif
