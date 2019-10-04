#include "eth_low.h"
#include "CfgUser.h"

mEth_t StmEth;

void mEth_ExtEvent(void) { eth_low_ExtEvent(&StmEth); }
void mEth_PhyEvent(void) { eth_low_PhyEvent(&StmEth); }

MadBool mEth_Init(void)
{
    mEth_InitData_t initData;
    
    GPIO_PinRemapConfig(GPIO_Remap_ETH, ENABLE);
    
    StmPIN_SetIO(&initData.RMII.MDC,    GPIOC, GPIO_Pin_1);
    StmPIN_SetIO(&initData.RMII.MDIO,   GPIOA, GPIO_Pin_2);
    StmPIN_SetIO(&initData.RMII.C50M,   GPIOA, GPIO_Pin_1);
    StmPIN_SetIO(&initData.RMII.TXEN,   GPIOB, GPIO_Pin_11);
    StmPIN_SetIO(&initData.RMII.TXD0,   GPIOB, GPIO_Pin_12);
    StmPIN_SetIO(&initData.RMII.TXD1,   GPIOB, GPIO_Pin_13);
    StmPIN_SetIO(&initData.RMII.CSR_DV, GPIOD, GPIO_Pin_8);
    StmPIN_SetIO(&initData.RMII.RXD0,   GPIOD, GPIO_Pin_9);
    StmPIN_SetIO(&initData.RMII.RXD1,   GPIOD, GPIO_Pin_10);
    StmPIN_SetIO(&initData.RMII.INTR,   GPIOD, GPIO_Pin_11);
    
    initData.Event.PORT              = GPIO_PortSourceGPIOD;
    initData.Event.LINE              = GPIO_PinSource11;
    initData.Event.EXIT.EXTI_Line    = EXTI_Line11;
    initData.Event.EXIT.EXTI_Mode    = EXTI_Mode_Interrupt;
    initData.Event.EXIT.EXTI_Trigger = EXTI_Trigger_Falling;
    initData.Event.IRQn              = EXTI15_10_IRQn;
    
    initData.Enable         = ENABLE;
    initData.PHY_ADDRESS    = 0x00;
#if 1
    do {
        MadU8 i;
        MadU8 *chip_id = madChipId();
        for(i=0; i<6; i++) {
            initData.MAC_ADDRESS[i] = chip_id[i];
        }
    } while(0);
#else
    initData.MAC_ADDRESS[0] = 0x4D;
    initData.MAC_ADDRESS[1] = 0x61;
    initData.MAC_ADDRESS[2] = 0x64;
    initData.MAC_ADDRESS[3] = 0x43;
    initData.MAC_ADDRESS[4] = 0x13;
    initData.MAC_ADDRESS[5] = 0x00;
#endif
    initData.Priority       = ISR_PRIO_ETH;
    initData.ThreadID       = THREAD_PRIO_DRIVER_ETH;
    initData.MaxPktSize     = ETH_MAX_PACKET_SIZE;
    initData.TxDscrNum      = mEth_TXBUFNB;
    initData.RxDscrNum      = mEth_RXBUFNB;

    madInstallExIrq(mEth_ExtEvent, EXTI15_10_IRQn);
    madInstallExIrq(mEth_PhyEvent, ETH_IRQn);
    
    if(eth_low_init(&StmEth, &initData)) { 
        return MTRUE; // Success
    }
    eth_init_failed(&StmEth);
    return MFALSE; // Failed
}
