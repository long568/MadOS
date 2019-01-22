#include "eth_low.h"

MadU32 uIP_dev_send(mEth_t *eth, MadU8 *buf, MadU32 len);
MadU32 uIP_dev_read(mEth_t *eth, MadU8 *buf);
MadU32 uIP_dev_rxsize(void);

inline MadU32 uIP_dev_send(mEth_t *eth, MadU8 *buf, MadU32 len) {
    if(eth->isLinked) {
        return ETH_HandleTxPkt(buf, len);
    } else {
        return 0;
    }
}

inline MadU32 uIP_dev_read(mEth_t *eth, MadU8 *buf) {
    MadU32 len;
    if(eth->isLinked) {
        len = ETH_HandleRxPkt(buf);
    } else {
        len = 0;
    }
    return len;
}

inline MadU32 uIP_dev_rxsize(void) {
    return ETH_GetRxPktSize();
}
