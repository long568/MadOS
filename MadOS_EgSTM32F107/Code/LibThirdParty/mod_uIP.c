#include "mod_uIP.h"
#if UIP_CORE_APP_DHCP
#include "dhcpc.h"
#endif

StmPIN eth_led; // For test

/*****************************************************
 *
 *  uIP-Misc
 *
 *****************************************************/
void uip_log(char *msg)
{  // Required by uip to print log.
    MAD_LOG("%s\n", msg);
}

void tcpip_output(void)
{   // Required by uip-split.c using for multiple network interface.
    // We have NOT used it yet.
    // Some say it equal to uip_fw_output();
}

/*****************************************************
 *
 *  uIP-Apps
 *
 *****************************************************/
#if UIP_CORE_APP_DHCP
void dhcpc_configured(const struct dhcpc_state *s)
{
    uip_sethostaddr(s->ipaddr);
    uip_setdraddr(s->default_router);
    uip_setnetmask(s->netmask);
}
#endif

/*****************************************************
 *
 *  uIP-Function
 *
 *****************************************************/
//void uIP_Send(const void *data, int len)
//{
//}

/*****************************************************
 *
 *  uIP-Core
 *
 *****************************************************/
clocker uIP_clocker;
timer   uIP_periodic_timer;
timer   uIP_arp_timer;

u8_t *uip_buf;
#define BUF ((struct uip_eth_hdr *)&uip_buf[0])

static void uIP_dev_send(mETH_t *eth);
static void uIP_dev_read(mETH_t *eth);
static MadBool uIP_preinit(mETH_t *eth);
static MadBool uIP_handler(mETH_t *eth, MadUint event, MadTim_t dt);

void uIP_Init(void)
{
    do { // For test
        StmPIN_SetIO(&eth_led, GPIOE, GPIO_Pin_0);
        StmPIN_DefInitOPP(&eth_led);
        StmPIN_SetHigh(&eth_led);
    } while(0);
    mEth_Init(uIP_preinit, uIP_handler);
}

void uIP_tcp_appcall(void)
{
}

void uIP_udp_appcall(void)
{
#if UIP_CORE_APP_DHCP
    dhcpc_appcall();
#endif
}

void uIP_linked_on(void)
{
    StmPIN_SetLow(&eth_led);
#if UIP_CORE_APP_DHCP    
    dhcpc_init(uip_ethaddr.addr, 6);
#endif
}

void uIP_linked_off(void)
{
    StmPIN_SetHigh(&eth_led);
#if UIP_CORE_APP_DHCP  
    dhcpc_deinit();
#endif
}

void uIP_dev_send(mETH_t *eth)
{
    if(eth->isLinked)
        ETH_HandleTxPkt(uip_buf, uip_len);
}

void uIP_dev_read(mETH_t *eth)
{
    uip_len = ETH_HandleRxPkt(uip_buf);
#if ETH_SOFT_FLOW_CONTROL
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

MadBool uIP_preinit(mETH_t *eth)
{
    MadUint i;
    uip_buf = (u8_t*)madMemMalloc(UIP_CONF_BUFFER_SIZE);
    if(!uip_buf) return MFALSE;
    clocker_init(&uIP_clocker);
    timer_init(&uIP_arp_timer);
    timer_init(&uIP_periodic_timer);
    timer_add(&uIP_arp_timer, &uIP_clocker);
    timer_add(&uIP_periodic_timer, &uIP_clocker);
    timer_set(&uIP_arp_timer, CLOCK_SECOND * 10);
    timer_set(&uIP_periodic_timer, CLOCK_SECOND / 2);
    uip_init();

    do {
        uip_ipaddr_t ipaddr;
#if !UIP_CORE_APP_DHCP
        uip_ipaddr(ipaddr, 192,168,1,156);
        uip_sethostaddr(ipaddr);
        uip_ipaddr(ipaddr, 192,168,1,1);
        uip_setdraddr(ipaddr);
        uip_ipaddr(ipaddr, 255,255,255,0);
        uip_setnetmask(ipaddr);
#else
        uip_ipaddr(ipaddr, 0,0,0,0);
        uip_sethostaddr(ipaddr);
        uip_setdraddr(ipaddr);
        uip_setnetmask(ipaddr);
#endif
    } while(0);
    for(i=0; i<6; i++) {
        uip_ethaddr.addr[i] = eth->MAC_ADDRESS[i];
    }
    return MTRUE;
}

MadBool uIP_handler(mETH_t *eth, MadUint event, MadTim_t dt)
{
    clocker_dt(&uIP_clocker, dt);
    
    if(event & EPE_STATUS_CHANGED) {
        if(eth->isLinked) uIP_linked_on();
        else              uIP_linked_off();
    }
    
    if(event & EPE_STATUS_RXPKT) {
        while(ETH_GetRxPktSize()) {
            uIP_dev_read(eth);
            if(uip_len > 0) {
                if(BUF->type == htons(UIP_ETHTYPE_IP)) {
                    uip_arp_ipin();
                    uip_input();
                    /* If the above function invocation resulted in data that
                       should be sent out on the network, the global variable
                       uip_len is set to a value > 0. */
                    if(uip_len > 0) {
                        uip_arp_out();
                        uIP_dev_send(eth);
                    }
                } else if(BUF->type == htons(UIP_ETHTYPE_ARP)) {
                    uip_arp_arpin();
                    /* If the above function invocation resulted in data that
                       should be sent out on the network, the global variable
                       uip_len is set to a value > 0. */
                    if(uip_len > 0) {
                        uIP_dev_send(eth);
                    }
                }
            }
        }
    } else if(timer_expired(&uIP_periodic_timer)) {
        int i;
        for(i = 0; i < UIP_CONNS; i++) {
            uip_periodic(i);
            /* If the above function invocation resulted in data that
            should be sent out on the network, the global variable
            uip_len is set to a value > 0. */
            if(uip_len > 0) {
                uip_arp_out();
                uIP_dev_send(eth);
            }
        }
#if UIP_UDP
        for(i = 0; i < UIP_UDP_CONNS; i++) {
            uip_udp_periodic(i);
            /* If the above function invocation resulted in data that
            should be sent out on the network, the global variable
            uip_len is set to a value > 0. */
            if(uip_len > 0) {
                uip_arp_out();
                uIP_dev_send(eth);
            }
        }
#endif /* UIP_UDP */
        /* Call the ARP timer function every 10 seconds. */
        if(timer_expired(&uIP_arp_timer)) {
            uip_arp_timer();
        }
    }
    
    return MTRUE;
}
