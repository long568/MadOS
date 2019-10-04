#include "eth_low.h"
#include "CfgUser.h"

static MadBool eth_low_init(mEth_t *eth, mEth_InitData_t *initData);
static MadBool eth_phy_init(mEth_t *eth);
static void    eth_init_failed(mEth_t *eth);
static MadBool eth_mac_deinit(mEth_t *eth);
static MadBool eth_mac_init(mEth_t *eth);
static MadBool eth_mac_start(mEth_t *eth);
static void    eth_driver_thread(MadVptr exData);

/******************************************
 *
 * For StmEth, instance of the only eth.
 *
 ******************************************/
static mEth_t StmEth;

//void ETH_WKUP_IRQHandler(void)
//void ETH_IRQHandler(void)

void mEth_ExtEvent(void)
{
    if(EXTI_GetITStatus(StmEth.EXIT_Line) != RESET) {
        madEventTrigger(&StmEth.Event, mEth_PE_STATUS_CHANGED);
        EXTI_ClearITPendingBit(StmEth.EXIT_Line);
    }
}

void mEth_PhyEvent(void)
{
    if(SET == ETH_GetDMAITStatus(ETH_DMA_IT_R)) {
        madEventTrigger(&StmEth.Event, mEth_PE_STATUS_RXPKT);
        ETH_DMAClearITPendingBit(ETH_DMA_IT_R);
    }
    ETH_DMAClearITPendingBit(ETH_DMA_IT_NIS);
}

MadBool mEth_Init(mEth_Preinit_t infn, mEth_Callback_t fn)
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
    initData.ThreadStkSize  = mEth_THREAD_STKSIZE;
    initData.MaxPktSize     = ETH_MAX_PACKET_SIZE;
    initData.TxDscrNum      = mEth_TXBUFNB;
    initData.RxDscrNum      = mEth_RXBUFNB;
    initData.infn           = infn;
    initData.fn             = fn;

    madInstallExIrq(mEth_ExtEvent, EXTI15_10_IRQn);
    madInstallExIrq(mEth_PhyEvent, ETH_IRQn);
    
    if(eth_low_init(&StmEth, &initData)) { 
        if(eth_phy_init(&StmEth)) {
            return MTRUE; // Success
        }
    }
    eth_init_failed(&StmEth);
    return MFALSE; // Failed
}

/******************************************
 *
 * For ETH-Class, Object-Oriented
 *
 ******************************************/

MadBool eth_low_init(mEth_t *eth, mEth_InitData_t *initData)
{
    MadUint i;
    MadU8 prio;
    FunctionalState  enable;
    NVIC_InitTypeDef NVIC_InitStructure;
    
    prio   = initData->Priority;
    enable = initData->Enable;
    
    eth->isLinked    = MFALSE;
    eth->ThreadID    = 0;
    eth->PHY_ADDRESS = initData->PHY_ADDRESS;
    for(i=0; i<6; i++)
        eth->MAC_ADDRESS[i] = initData->MAC_ADDRESS[i];
    eth->EXIT_Line   = initData->Event.EXIT.EXTI_Line;
    eth->INTP.port   = initData->RMII.INTR.port;
    eth->INTP.pin    = initData->RMII.INTR.pin;
    eth->MaxPktSize  = initData->MaxPktSize;
    eth->TxDscrNum   = initData->TxDscrNum;
    eth->RxDscrNum   = initData->RxDscrNum;

    eth->TxDscr      = (ETH_DMADESCTypeDef*)madMemMalloc(eth->TxDscrNum * sizeof(ETH_DMADESCTypeDef));
    eth->RxDscr      = (ETH_DMADESCTypeDef*)madMemMalloc(eth->RxDscrNum * sizeof(ETH_DMADESCTypeDef));
    eth->TxBuff      = (MadU8*)madMemMalloc(eth->TxDscrNum * eth->MaxPktSize);
    eth->RxBuff      = (MadU8*)madMemMalloc(eth->RxDscrNum * eth->MaxPktSize);
    if((!eth->TxDscr) || (!eth->RxDscr) || (!eth->TxBuff) || (!eth->RxBuff)) {
        return MFALSE;
    }
    eth->Event = madEventCreate(mEth_PE_STATUS_ALL, MEMODE_WAIT_ONE, MEOPT_DELAY);
    if(MNULL == eth->Event) {
        return MFALSE;
    }
    if(initData->infn(eth) &&
       madThreadCreate(eth_driver_thread, eth, initData->ThreadStkSize, initData->ThreadID)) {
        eth->ThreadID = initData->ThreadID;
        eth->fn       = initData->fn;
    } else {
        return MFALSE;
    }
    
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_ETH_MAC | 
                          RCC_AHBPeriph_ETH_MAC_Tx | 
                          RCC_AHBPeriph_ETH_MAC_Rx, 
                          ENABLE);
    
    NVIC_InitStructure.NVIC_IRQChannel = ETH_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = prio;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = enable;
    NVIC_Init(&NVIC_InitStructure);
    
    StmPIN_DefInitAPP(&initData->RMII.MDC);
    StmPIN_DefInitAPP(&initData->RMII.MDIO);
    StmPIN_DefInitIFL(&initData->RMII.C50M);
    StmPIN_DefInitAPP(&initData->RMII.TXEN);
    StmPIN_DefInitAPP(&initData->RMII.TXD0);
    StmPIN_DefInitAPP(&initData->RMII.TXD1);
    StmPIN_DefInitIFL(&initData->RMII.CSR_DV);
    StmPIN_DefInitIFL(&initData->RMII.RXD0);
    StmPIN_DefInitIFL(&initData->RMII.RXD1);
    StmPIN_DefInitIFL(&initData->RMII.INTR);
    
    GPIO_EXTILineConfig(initData->Event.PORT, initData->Event.LINE);
    initData->Event.EXIT.EXTI_LineCmd = enable;
    EXTI_Init(&initData->Event.EXIT);
    NVIC_InitStructure.NVIC_IRQChannel = initData->Event.IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = prio;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = enable;
    NVIC_Init(&NVIC_InitStructure);
    
    GPIO_ETH_MediaInterfaceConfig(GPIO_ETH_MediaInterface_RMII);
    return MTRUE;
}

void eth_init_failed(mEth_t *eth)
{
    if(eth) {
        madMemFree(eth->TxDscr);
        madMemFree(eth->RxDscr);
        madMemFree(eth->TxBuff);
        madMemFree(eth->RxBuff);
        madEventDelete(&StmEth.Event);
    }
}

MadBool eth_phy_init(mEth_t *eth)
{
    MadU16 phy_reg;
    MadU16 phy_addr;
    phy_addr = eth->PHY_ADDRESS;
    // Get PHY-State to make sure IP101A is working.
    mEth_PHY_WT(ETH_ReadPHYRegister(phy_addr, mEth_PR_CTRL), mEth_TIMEOUT_TICKS);
    // Reset PHY
    phy_reg = ETH_ReadPHYRegister(phy_addr, mEth_PR_CTRL);
    phy_reg |= PHY_Reset;
    ETH_WritePHYRegister(phy_addr, mEth_PR_CTRL, phy_reg);
    mEth_PHY_WF(ETH_ReadPHYRegister(phy_addr, mEth_PR_CTRL) & PHY_Reset, mEth_TIMEOUT_TICKS);
    _eth_delay_(PHY_ResetDelay);
    // Configure interrupt of IP101A
    ETH_WritePHYRegister(phy_addr, mEth_PR_INTR, mEth_PF_ISR);
    ETH_ReadPHYRegister(phy_addr, mEth_PR_INTR);
    return MTRUE;
}

MadBool eth_mac_deinit(mEth_t *eth)
{
    ETH_DeInit();
    ETH_SoftwareReset();
    while (ETH_GetSoftwareResetStatus() == SET);
    eth->isLinked = MFALSE;
    return MTRUE;
}

MadBool eth_mac_init(mEth_t *eth)
{
    MadU16 phy_addr;
    ETH_InitTypeDef ETH_InitStructure;
    
    phy_addr = eth->PHY_ADDRESS;
    ETH_StructInit(&ETH_InitStructure);
    
    /* Fill ETH_InitStructure parametrs */
    /*------------------------   MAC   -----------------------------------*/
    ETH_InitStructure.ETH_AutoNegotiation = ETH_AutoNegotiation_Enable  ;
    ETH_InitStructure.ETH_LoopbackMode = ETH_LoopbackMode_Disable;
    ETH_InitStructure.ETH_RetryTransmission = ETH_RetryTransmission_Disable;
    ETH_InitStructure.ETH_AutomaticPadCRCStrip = ETH_AutomaticPadCRCStrip_Disable;
    ETH_InitStructure.ETH_ReceiveAll = ETH_ReceiveAll_Disable;
    ETH_InitStructure.ETH_BroadcastFramesReception = ETH_BroadcastFramesReception_Enable;
    ETH_InitStructure.ETH_PromiscuousMode = ETH_PromiscuousMode_Disable;
    ETH_InitStructure.ETH_MulticastFramesFilter = ETH_MulticastFramesFilter_Perfect;
    ETH_InitStructure.ETH_UnicastFramesFilter = ETH_UnicastFramesFilter_Perfect;
#if  mEth_CHECKSUM_BY_HARDWARE
    ETH_InitStructure.ETH_ChecksumOffload = ETH_ChecksumOffload_Enable;
#endif
    
    /*------------------------   DMA   -----------------------------------*/
    /* When we use the Checksum offload feature, we need to enable the Store and Forward mode: 
    the store and forward guarantee that a whole frame is stored in the FIFO, so the MAC can insert/verify the checksum, 
    if the checksum is OK the DMA can handle the frame otherwise the frame is dropped */
    ETH_InitStructure.ETH_DropTCPIPChecksumErrorFrame = ETH_DropTCPIPChecksumErrorFrame_Enable; 
    ETH_InitStructure.ETH_ReceiveStoreForward = ETH_ReceiveStoreForward_Enable;         
    ETH_InitStructure.ETH_TransmitStoreForward = ETH_TransmitStoreForward_Enable;     
    ETH_InitStructure.ETH_ForwardErrorFrames = ETH_ForwardErrorFrames_Disable;       
    ETH_InitStructure.ETH_ForwardUndersizedGoodFrames = ETH_ForwardUndersizedGoodFrames_Disable;   
    ETH_InitStructure.ETH_SecondFrameOperate = ETH_SecondFrameOperate_Enable;                                                          
    ETH_InitStructure.ETH_AddressAlignedBeats = ETH_AddressAlignedBeats_Enable;      
    ETH_InitStructure.ETH_FixedBurst = ETH_FixedBurst_Enable;                
    ETH_InitStructure.ETH_RxDMABurstLength = ETH_RxDMABurstLength_32Beat;
    ETH_InitStructure.ETH_TxDMABurstLength = ETH_TxDMABurstLength_32Beat;
    ETH_InitStructure.ETH_DMAArbitration = ETH_DMAArbitration_RoundRobin_RxTx_2_1;

    if(ETH_Init(&ETH_InitStructure, phy_addr)) {
        ETH_DMAITConfig(ETH_DMA_IT_NIS | ETH_DMA_IT_R, ENABLE);
        ETH_MACAddressConfig(ETH_MAC_Address0, eth->MAC_ADDRESS);
        eth_mac_start(eth);
        eth->isLinked = MTRUE;
        return MTRUE;
    } else {
        return MFALSE;
    }
}

MadBool eth_mac_start(mEth_t *eth)
{
    int i;
    ETH_DMATxDescChainInit(eth->TxDscr, eth->TxBuff, eth->TxDscrNum);
    ETH_DMARxDescChainInit(eth->RxDscr, eth->RxBuff, eth->RxDscrNum);
    for(i=0; i<eth->RxDscrNum; i++) {
        ETH_DMARxDescReceiveITConfig(&eth->RxDscr[i], ENABLE);
    }
#if  mEth_CHECKSUM_BY_HARDWARE
    for(i=0; i<eth->TxDscrNum; i++) {
        ETH_DMATxDescChecksumInsertionConfig(&eth->TxDscr[i], ETH_DMATxDesc_ChecksumTCPUDPICMPFull);
    }
#endif
    ETH_Start();
    return MTRUE;
}

void eth_driver_thread(MadVptr exData)
{
    MadU8 ok;
    MadUint event;
    MadTim_t dt;
    mEth_t *eth = (mEth_t*)exData;

    event = mEth_PE_STATUS_CHANGED;
    dt    = 0;
    
    while(1) {
        ok = madEventWait(&eth->Event, &event, mEth_EVENT_TIMEOUT);
        switch(ok) {
            case MAD_ERR_OK: {
                MadCpsr_t cpsr;
                MadTim_t  remain;
                madEnterCritical(cpsr);
                remain = MadCurTCB->timeCntRemain;
                madExitCritical(cpsr);
                dt = mEth_EVENT_TIMEOUT - remain;
                break;
            }
            case MAD_ERR_TIMEOUT:
                event = mEth_PE_STATUS_TIMEOUT;
                dt = mEth_EVENT_TIMEOUT;
                break;
            default:
                event = 0;
                dt    = 0;
                break;
        }
        
        // Handle PHY-Event
        if(event & mEth_PE_STATUS_CHANGED) {
            madTimeDly(10);
            dt += 10;
            if(0 == StmPIN_ReadInValue(&eth->INTP)) {
                MadU16 phy_reg;
                MadU16 phy_addr = eth->PHY_ADDRESS;
                // Read 2 times to make sure we got real value of the reg.
                // This maybe a bug of IP101A
                ETH_ReadPHYRegister(phy_addr, mEth_PR_INTR);
                ETH_ReadPHYRegister(phy_addr, mEth_PR_INTR);
                ETH_ReadPHYRegister(phy_addr, mEth_PR_STAT);
                phy_reg = ETH_ReadPHYRegister(phy_addr, mEth_PR_STAT);
                eth_mac_deinit(eth);
                if(phy_reg & PHY_Linked_Status) {
                    eth_mac_init(eth);
                }
            }
        }
        
        // Handle App
        if(eth->fn) eth->fn(eth, event, dt);
    }
}
