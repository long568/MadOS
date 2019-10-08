#include "eth_low.h"

static void eth_driver_thread(MadVptr exData);

//void ETH_WKUP_IRQHandler(void)
//void ETH_IRQHandler(void)

void eth_low_ExtEvent(mEth_t *eth)
{
    if(EXTI_GetITStatus(eth->EXIT_Line) != RESET) {
        madEventTrigger(&eth->Event, mEth_PE_STATUS_CHANGED);
        EXTI_ClearITPendingBit(eth->EXIT_Line);
    }
}

void eth_low_PhyEvent(mEth_t *eth)
{
    if(SET == ETH_GetDMAITStatus(ETH_DMA_IT_R)) {
        madEventTrigger(&eth->Event, mEth_PE_STATUS_RXPKT);
        ETH_DMAClearITPendingBit(ETH_DMA_IT_R);
    }
    ETH_DMAClearITPendingBit(ETH_DMA_IT_NIS);
}

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
    eth->fn          = 0;
    eth->ep          = initData->ep;
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
        madInstallExIrq(initData->extIRQh, initData->extIRQn);
        madInstallExIrq(initData->ethIRQh, initData->ethIRQn);
        eth->ThreadID = initData->ThreadID;
        eth->fn       = initData->cbfn;
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

    return eth_phy_init(eth);
}

void eth_init_failed(mEth_t *eth)
{
    if(eth) {
        eth_mac_deinit(eth);
        madMemFree(eth->TxDscr);
        madMemFree(eth->RxDscr);
        madMemFree(eth->TxBuff);
        madMemFree(eth->RxBuff);
        madEventDelete(&eth->Event);
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
#if 0
    _eth_delay_(PHY_ResetDelay);
    return eth_mac_init(eth);
#else
    return MTRUE;
#endif
}

MadBool eth_mac_deinit(mEth_t *eth)
{
    ETH_DeInit();
    ETH_SoftwareReset();
    while (ETH_GetSoftwareResetStatus() == SET);
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
    ETH_InitStructure.ETH_DMAArbitration = ETH_DMAArbitration_RoundRobin_RxTx_1_1; //ETH_DMAArbitration_RoundRobin_RxTx_2_1;

    if(ETH_Init(&ETH_InitStructure, phy_addr)) {
        ETH_DMAITConfig(ETH_DMA_IT_NIS | ETH_DMA_IT_R, ENABLE);
        ETH_MACAddressConfig(ETH_MAC_Address0, eth->MAC_ADDRESS);
        eth_mac_start(eth);
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
            int i;
			for(i=0; i<4; i++) {
				madTimeDly(5);
                dt += 5;
				if(0 != StmPIN_ReadInValue(&eth->INTP)) {
                    event &= ~mEth_PE_STATUS_CHANGED;
					break;
				}
			}
            if(event & mEth_PE_STATUS_CHANGED) {
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
                    eth->isLinked = MTRUE;
                    MAD_LOG("[ETH] Link up\n");
                } else {
                    eth->isLinked = MFALSE;
                    MAD_LOG("[ETH] Link down\n");
                }
            }
        }
        
        // Handle App
        if(eth->fn) eth->fn(eth, event, dt);
    }
}
