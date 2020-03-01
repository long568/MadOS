#ifndef __ETH_LOW__H__
#define __ETH_LOW__H__

#include "eth_low_cfg.h"

void    eth_low_deinit  (mEth_t *eth);
MadBool eth_low_init    (mEth_t *eth, const mEth_InitData_t *initData, mEth_Callback_t fn, MadVptr ep);
MadBool eth_phy_init    (mEth_t *eth);
MadBool eth_mac_deinit  (mEth_t *eth);
MadBool eth_mac_init    (mEth_t *eth);
MadBool eth_mac_start   (mEth_t *eth);
void    eth_low_ExtEvent(mEth_t *eth);
void    eth_low_PhyEvent(mEth_t *eth);

MadBool eth_port_init   (mEth_t *eth);

#endif
