#include "eth_low.h"

MadU32 uIP_dev_send(mEth_t *eth, MadU8 *buf, MadU16 len)
{
    MadU32 res = 0;
    uint8_t *txbuf;
    if(eth->isLinked && ETH_TxPktRdy() > 0) {
        txbuf = (uint8_t *)ETH_GetCurrentTxBuffer();
        memcpy(txbuf, buf, len);
        ETH_TxPkt_ChainMode(len);
        res = len;
    }
    return res;
}

MadU32 uIP_dev_read(mEth_t *eth, MadU8 *buf, MadU16 *len)
{
    (void)eth;
    uint32_t rxlen;
    uint8_t *rxbuf;
    FrameTypeDef frame;
    frame = ETH_RxPkt_ChainMode();
    rxlen = frame.length;
    if(len) *len = (MadU16)rxlen;
    if(rxlen == 0) return 0;
    rxbuf = (uint8_t *)frame.buffer;
    memcpy(buf, rxbuf, rxlen);
    ETH_RxPktResume(frame.descriptor);
    return rxlen;
}
