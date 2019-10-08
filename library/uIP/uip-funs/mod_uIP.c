/*
 * Test Cmd
 * psping -i 0 -l 1234 -q -t 192.168.1.109
 */
#include <stdlib.h>
#include "mod_uIP.h"

clocker    uIP_Clocker;
MadBool    uIP_is_linked;
MadBool    uIP_is_configured;

timer      uIP_periodic_timer;
timer      uIP_arp_timer;
u8_t       *uip_buf;
MadSemCB_t *uIP_locker;

#define APPCONN_CALL(x) \
do {                                          \
    if(x && x->appstate.app_call)             \
        x->appstate.app_call(x->appstate.ep); \
} while(0)

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
 *  uIP Core Function
 *
 *****************************************************/
inline void uIP_Lock(void) {
    madSemWait(&uIP_locker, 0);
}

inline void uIP_Unlock(void) {
    madSemRelease(&uIP_locker);
}

/*****************************************************
 *
 *  uIP Core Apps Callback
 *
 *****************************************************/
#if UIP_CORE_APP_DNS
void resolv_found(char *name, u16_t *ipaddr) { 
    int i;
    for(i = 0; i < UIP_CONNS; i++) {
        register struct uip_conn *conn = &uip_conns[i];
        if(conn->appstate.status == uIP_CONN_WORKING && conn->appstate.dns_call){
            conn->appstate.dns_call(conn->appstate.ep, name, ipaddr);
        }
    }
    for(i = 0; i < UIP_UDP_CONNS; i++) {
        register struct uip_udp_conn *conn = &uip_udp_conns[i];
        if(conn->appstate.status == uIP_CONN_WORKING && conn->appstate.dns_call){
            conn->appstate.dns_call(conn->appstate.ep, name, ipaddr);
        }
    }
}
#endif /* UIP_CORE_APP_DNS */

/*****************************************************
 *
 *  uIP Core Callback
 *
 *****************************************************/
void uIP_tcp_appcall(void) { APPCONN_CALL(uip_conn); }
void uIP_udp_appcall(void) { APPCONN_CALL(uip_udp_conn); }

/*****************************************************
 *
 *  uIP-Core
 *
 *****************************************************/
#define BUF ((struct uip_eth_hdr *)&uip_buf[0])

inline MadBool uIP_Init(void) {
    return mEth_Init(uIP_preinit, uIP_handler, 0);
}

MadBool uIP_preinit(mEth_t *eth)
{
    MadUint i;
    uip_buf = (u8_t*)malloc(UIP_CONF_BUFFER_SIZE);
    uIP_locker = madSemCreate(1);
    if((!uip_buf) || (!uIP_locker)) {
        free(uip_buf);
        madSemDelete(&uIP_locker);
        return MFALSE;
    }
    uIP_Lock();
    uIP_is_linked = MFALSE;
    uIP_is_configured = MFALSE;
    clocker_init(&uIP_Clocker);
    timer_init(&uIP_arp_timer);
    timer_init(&uIP_periodic_timer);
    timer_add(&uIP_arp_timer, &uIP_Clocker);
    timer_add(&uIP_periodic_timer, &uIP_Clocker);
    timer_set(&uIP_arp_timer, MadTicksPerSec * 10);
    timer_set(&uIP_periodic_timer, MadTicksPerSec / 2);
    uip_init();
    for(i=0; i<6; i++)
        uip_ethaddr.addr[i] = eth->MAC_ADDRESS[i];
    uIP_Unlock();
    do {
        uip_ipaddr_t ipaddr;
#if UIP_CORE_APP_DHCP
        uIP_Lock();
        uip_ipaddr(ipaddr, 0,0,0,0);
        uip_sethostaddr(ipaddr);
        uip_setdraddr(ipaddr);
        uip_setnetmask(ipaddr);
        uip_setdnsaddr(ipaddr);
        uIP_Unlock();
        dhcpc_init();
#else
        uIP_Lock();
        uip_ipaddr(ipaddr, 192,168,1,235);
        uip_sethostaddr(ipaddr);
        uip_ipaddr(ipaddr, 192,168,1,1);
        uip_setdraddr(ipaddr);
        uip_setdnsaddr(ipaddr);
        uip_ipaddr(ipaddr, 255,255,255,0);
        uip_setnetmask(ipaddr);
        uIP_is_configured = MTRUE;
        uIP_Unlock();
#endif
#if UIP_CORE_APP_DNS
        resolv_init();
#endif
    } while(0);
    return MTRUE;
}

MadBool uIP_handler(mEth_t *eth, MadUint event, MadTim_t dt)
{
    uIP_Lock();
    clocker_dt(&uIP_Clocker, dt);

    if(event & mEth_PE_STATUS_CHANGED) {
        uIP_is_linked = eth->isLinked;
    }
    
    if(event & mEth_PE_STATUS_RXPKT) {
        while(uIP_dev_read(eth, uip_buf, &uip_len) > 0) {
            if(BUF->type == htons(UIP_ETHTYPE_IP)) {
                uip_arp_ipin();
                uip_input();
                /* If the above function invocation resulted in data that
                    should be sent out on the network, the global variable
                    uip_len is set to a value > 0. */
                if(uip_len > 0) {
                    uip_arp_out();
                    uIP_dev_send(eth, uip_buf, uip_len);
                }
            } else if(BUF->type == htons(UIP_ETHTYPE_ARP)) {
                uip_arp_arpin();
                /* If the above function invocation resulted in data that
                    should be sent out on the network, the global variable
                    uip_len is set to a value > 0. */
                if(uip_len > 0) {
                    uIP_dev_send(eth, uip_buf, uip_len);
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
                uIP_dev_send(eth, uip_buf, uip_len);
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
                uIP_dev_send(eth, uip_buf, uip_len);
            }
        }
#endif /* UIP_UDP */
        /* Call the ARP timer function every 10 seconds. */
        if(timer_expired(&uIP_arp_timer)) {
            uip_arp_timer();
        }
    }
    
    uIP_Unlock();
    return MTRUE;
}
