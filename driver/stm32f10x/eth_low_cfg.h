#ifndef __ETH_LOW_CFG__H__
#define __ETH_LOW_CFG__H__

#include "stm32_eth.h"

#define mEth_CHECKSUM_BY_HARDWARE 1
#define mEth_SOFT_FLOW_CONTROL    0

#define mEth_TIMEOUT_TICKS  (888) // ticks
#define mEth_EVENT_TIMEOUT  (100) // ms
#define mEth_THREAD_STKSIZE (2 * 1024)
#define mEth_TXBUFNB        ((MadU8)2)
#define mEth_RXBUFNB        ((MadU8)2)

#define mEth_PHY_WT(c, t) do { MadUint n = 0; while((!(c)) && (++n < (t))); if(n == (t)) return MFALSE; } while(0)
#define mEth_PHY_WF(c, t) do { MadUint n = 0; while(  (c)  && (++n < (t))); if(n == (t)) return MFALSE; } while(0)

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
    mEth_Callback_t fn;
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
#if mEth_SOFT_FLOW_CONTROL
    MadU8              RxDscrCnt;
#endif
    MadEventCB_t       *Event;
    mEth_Callback_t    fn;
    ETH_DMADESCTypeDef *TxDscr;
    ETH_DMADESCTypeDef *RxDscr;
    MadU8              *TxBuff;
    MadU8              *RxBuff;
} mEth_t;

#endif
