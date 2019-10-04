#ifndef __ETH_LOW__H__
#define __ETH_LOW__H__

#include "eth_low_cfg.h"

#define ethernetif _mEth_t // For lwip

MadBool eth_low_init    (mEth_t *eth, mEth_InitData_t *initData);
MadBool eth_phy_init    (mEth_t *eth);
void    eth_init_failed (mEth_t *eth);
MadBool eth_mac_deinit  (mEth_t *eth);
MadBool eth_mac_init    (mEth_t *eth);
MadBool eth_mac_start   (mEth_t *eth);
void    eth_low_ExtEvent(mEth_t *eth);
void    eth_low_PhyEvent(mEth_t *eth);

extern mEth_t StmEth;
extern MadBool mEth_Init(void);

#endif
