#ifndef __ETH_LOW_REG__H__
#define __ETH_LOW_REG__H__

#include "stm32_eth.h"
#include "Stm32Tools.h"

typedef enum {
    mEth_PR_CTRL = 0,
    mEth_PR_STAT,
    mEth_PR_ID1,
    mEth_PR_ID2,
    mEth_PR_ANAR,
    mEth_PR_ANLPAR,
    mEth_PR_ANER,
#ifdef USE_IP101A
    mEth_PR_SPECTRL = 16,
    mEth_PR_INTR,
#endif
} mEth_PHY_REG;

#ifdef USE_IP101A
typedef enum {
    mEth_PF_ISR = 0x8600, // 1000_0110_0000_0000
} mEth_PHY_FLAG;
#endif

#endif
