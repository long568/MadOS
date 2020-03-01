#include <stdarg.h>
#include <stdio.h>
#include "MadDev.h"
#include "eth_low.h"
#include "CfgUser.h"

static mEth_t port;

static void mEth_ExtEvent(void) { eth_low_ExtEvent(&port); }
static void mEth_PhyEvent(void) { eth_low_PhyEvent(&port); }

static const mEth_InitData_t LowArgs = {
    { // RMII
        {GPIOC, GPIO_Pin_1},  // MDC
        {GPIOA, GPIO_Pin_2},  // MDIO
        {GPIOA, GPIO_Pin_1},  // C50M
        {GPIOB, GPIO_Pin_11}, // TXEN
        {GPIOB, GPIO_Pin_12}, // TXD0
        {GPIOB, GPIO_Pin_13}, // TXD1
        {GPIOD, GPIO_Pin_8},  // CSR_DV
        {GPIOD, GPIO_Pin_9},  // RXD0
        {GPIOD, GPIO_Pin_10}, // RXD1
        {GPIOD, GPIO_Pin_11}  // INTR
    },
    { // Event
        GPIO_PortSourceGPIOD,
        GPIO_PinSource11,
        {
            EXTI_Line11,
            EXTI_Mode_Interrupt,
            EXTI_Trigger_Falling,
        },
        mEth_ExtEvent,
        EXTI15_10_IRQn
    },
    0x00,
    { 0 },
    MTRUE,
    ISR_PRIO_ETH,
    THREAD_PRIO_DRIVER_ETH,
    mEth_THREAD_STKSIZE,
    ETH_MAX_PACKET_SIZE,
    mEth_TXBUFNB,
    mEth_RXBUFNB,    
    mEth_PhyEvent,
    ETH_IRQn
};

static const MadDevArgs_t Args = {
    0,
    0,
    0,
    &LowArgs
};

MadDev_t Eth0 = { "eth0", &port, &Args, &MadDrvEther, NULL };
