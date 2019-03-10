#ifndef __MAD_DRV_SDHC__H__
#define __MAD_DRV_SDHC__H__

#define SECTOR_ROLL       9
#define SECTOR_SIZE       (1 << SECTOR_ROLL)
#define OPT_RETRY_NUM     10000
#define CMD_TIME_OUT      600
#define DAT_TIME_OUT      1000

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
