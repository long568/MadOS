#include "eth_low.h"

MadU32 uIP_dev_send  (mEth_t *eth, MadU8 *buf, MadU32 len);
MadU32 uIP_dev_read  (mEth_t *eth, MadU8 *buf);
MadU32 uIP_dev_read2 (mEth_t *eth, MadU8 **pbuf);
MadU32 uIP_dev_rxsize(mEth_t *eth);

inline MadU32 uIP_dev_send(mEth_t *eth, MadU8 *buf, MadU32 len) {
#if 0
    if(eth->isLinked) {
        return ETH_HandleTxPkt(buf, len);
    } else {
        return 0;
    }
#else
    (void)eth;
    // return ETH_HandleTxPkt(buf, len);
    return ETH_HandleTxPkt2(buf, len);
#endif
}

inline MadU32 uIP_dev_read(mEth_t *eth, MadU8 *buf) {
    (void)eth;
    return ETH_HandleRxPkt(buf);
}

inline MadU32 uIP_dev_read2(mEth_t *eth, MadU8 **pbuf) {
    (void)eth;
    return ETH_HandleRxPkt2(pbuf);
}

inline MadU32 uIP_dev_rxsize(mEth_t *eth) {
    (void)eth;
    return ETH_GetRxPktSize();
}
