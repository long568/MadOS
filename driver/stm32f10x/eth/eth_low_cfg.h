#ifndef __ETH_LOW_CFG__H__
#define __ETH_LOW_CFG__H__

#include "eth_low_reg.h"

#define mEth_CHECKSUM_BY_HARDWARE 1

#define mEth_EVENT_TIMEOUT  (100) // LwIP: 0, uIP: 100
#define mEth_TIMEOUT_TICKS  (888)
#define mEth_THREAD_STKSIZE (1 * 1024)
#define mEth_TXBUFNB        ((MadU8)2)
#define mEth_RXBUFNB        ((MadU8)2)

#define mEth_PHY_WT(c, t) do { MadUint n = 0; while((!(c)) && (++n < (t))); if(n == (t)) return MFALSE; } while(0)
#define mEth_PHY_WF(c, t) do { MadUint n = 0; while(  (c)  && (++n < (t))); if(n == (t)) return MFALSE; } while(0)

typedef enum {
    mEth_PE_STATUS_CHANGED = 0x0001,
    mEth_PE_STATUS_TIMEOUT = 0x0002,
    mEth_PE_STATUS_RXPKT   = 0x0004,
    mEth_PE_STATUS_TXPKT   = 0x0008,
    mEth_PE_STATUS_ALL     = mEth_PE_STATUS_CHANGED | mEth_PE_STATUS_RXPKT | mEth_PE_STATUS_TIMEOUT,
} mEth_PHY_EVENT;

struct _mEth_InitData_t;
struct _mEth_t;

typedef MadBool(*mEth_Preinit_t) (struct _mEth_t *eth);
typedef MadBool(*mEth_Callback_t)(struct _mEth_t *eth, MadUint event, MadTim_t dt);

typedef struct _mEth_InitData_t {
    struct {
        StmPIN MDC;
        StmPIN MDIO;
        StmPIN C50M;
        StmPIN TXEN;
        StmPIN TXD0;
        StmPIN TXD1;
        StmPIN CSR_DV;
        StmPIN RXD0;
        StmPIN RXD1;
        StmPIN INTR;
    } RMII;
    struct {
        MadU8            PORT;
        MadU8            LINE;
        EXTI_InitTypeDef EXIT;
        IRQn_Type        IRQn;
    } Event;
    FunctionalState Enable;
    MadU16          PHY_ADDRESS;
    MadU8           MAC_ADDRESS[6];
    MadU8           Priority;
    MadU8           ThreadID;
    MadSize_t       ThreadStkSize;
    MadU16          MaxPktSize;
    MadU8           TxDscrNum;
    MadU8           RxDscrNum;
    mEth_Preinit_t  infn;
    mEth_Callback_t cbfn;
    xIRQ_Handler    extIRQh;
    xIRQ_Handler    ethIRQh;
    MadU32          extIRQn;
    MadU32          ethIRQn;
    MadVptr         ep;
} mEth_InitData_t;

typedef struct _mEth_t {
    MadBool            isLinked;
    MadU8              ThreadID;
    MadU16             PHY_ADDRESS;
    MadU8              MAC_ADDRESS[6];
    MadU32             EXIT_Line;
    StmPIN             INTP;
    MadU16             MaxPktSize;
    MadU8              TxDscrNum;
    MadU8              RxDscrNum;
    mEth_Callback_t    fn;
    MadEventCB_t       *Event;
    ETH_DMADESCTypeDef *TxDscr;
    ETH_DMADESCTypeDef *RxDscr;
    MadU8              *TxBuff;
    MadU8              *RxBuff;
    MadVptr            ep;
} mEth_t;

#endif
