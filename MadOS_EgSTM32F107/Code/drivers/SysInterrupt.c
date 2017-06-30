extern void EXTI15_10_IRQHandler(void);
extern void ETH_IRQHandler(void);

extern void mETH_PhyEvent(void);
void EXTI15_10_IRQHandler(void)
{
    mETH_PhyEvent();
}
