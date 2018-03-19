extern void mETH_ExtEvent(void);
extern void mETH_PhyEvent(void);

void EXTI15_10_IRQHandler(void)
{
	mETH_ExtEvent();
}

void ETH_IRQHandler(void)
{
    mETH_PhyEvent();
}
