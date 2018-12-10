#include "mod_uIP.h"

inline MadBool uIP_Init(void) {
    return mEth_Init(uIP_preinit, uIP_handler);
}

inline void uIP_dev_send(mEth_t *eth) {
    if(eth->isLinked)
        ETH_HandleTxPkt(uip_buf, uip_len);
}

inline void uIP_dev_read(mEth_t *eth) {
    uip_len = ETH_HandleRxPkt(uip_buf);
#if mEth_SOFT_FLOW_CONTROL
    do {
        MadCpsr_t cpsr;
        madEnterCritical(cpsr);
        if(eth->RxDscrCnt-- == eth->RxDscrNum) {
            ETH_DMAReceptionCmd(ENABLE);
        }
        madExitCritical(cpsr);
    } while(0);
#else
    (void)eth;
#endif
}

inline uint32_t uIP_dev_rxsize(void) {
    return ETH_GetRxPktSize();
}
