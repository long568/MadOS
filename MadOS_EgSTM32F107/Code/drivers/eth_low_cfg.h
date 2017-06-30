#ifndef __ETH_LOW_CFG__H__
#define __ETH_LOW_CFG__H__

#include "stm32_eth.h"

#define ETH_CHECKSUM_BY_HARDWARE 1
#define ETH_SOFT_FLOW_CONTROL    0

#define ETH_TIMEOUT_TICKS  (888) // ticks
#define ETH_EVENT_TIMEOUT  (100) // ms
#define ETH_THREAD_STKSIZE (2 * 1024)
#define ETH_TXBUFNB        ((MadU8)2)
#define ETH_RXBUFNB        ((MadU8)2)

#define ETH_PHY_WT(c, t) do { MadUint n = 0; while((!(c)) && (++n < (t))); if(n == (t)) return MFALSE; } while(0)
#define ETH_PHY_WF(c, t) do { MadUint n = 0; while(  (c)  && (++n < (t))); if(n == (t)) return MFALSE; } while(0)

typedef enum {
    EPR_CTRL = 0,
    EPR_STAT,
    EPR_ID1,
    EPR_ID2,
    EPR_ANAR,
    EPR_ANLPAR,
    EPR_ANER,
#ifdef USE_IP101A
    EPR_SPECTRL = 16,
    EPR_INTR,
#endif
} ETH_PHY_REG;

#ifdef USE_IP101A
typedef enum {
    EPF_ISR = 0x8600, // 1000_0110_0000_0000
} ETH_PHY_FLAG;
#endif

typedef enum {
    EPE_STATUS_CHANGED = 0x0001,
    EPE_STATUS_TIMEOUT = 0x0002,
    EPE_STATUS_RXPKT   = 0x0004,
    EPE_STATUS_TXPKT   = 0x0008,
    EPE_STATUS_ALL     = EPE_STATUS_CHANGED | EPE_STATUS_RXPKT | EPE_STATUS_TIMEOUT,
} ETH_PHY_EVENT;

struct _mETH_InitData;
struct _mETH_t;

typedef MadBool(*mETH_Preinit) (struct _mETH_t *eth);
typedef MadBool(*mETH_Callback)(struct _mETH_t *eth, MadUint event, MadTim_t dt);

typedef struct _mETH_InitData {
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
    mETH_Preinit    infn;
    mETH_Callback   fn;
} mETH_InitData;

typedef struct _mETH_t {
    MadBool            isLinked;
    MadU8              ThreadID;
    MadU16             PHY_ADDRESS;
    MadU8              MAC_ADDRESS[6];
    MadU32             EXIT_Line;
    StmPIN             INTP;
    MadU16             MaxPktSize;
    MadU8              TxDscrNum;
    MadU8              RxDscrNum;
#if ETH_SOFT_FLOW_CONTROL
    MadU8              RxDscrCnt;
#endif
    MadEventCB_t       *Event;
    mETH_Callback      fn;
    ETH_DMADESCTypeDef *TxDscr;
    ETH_DMADESCTypeDef *RxDscr;
    MadU8              *TxBuff;
    MadU8              *RxBuff;
} mETH_t;

extern mETH_t StmEth;

#endif
