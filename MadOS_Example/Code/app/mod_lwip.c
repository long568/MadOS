#include "mod_lwip.h"

extern err_t ethernetif_init(struct netif *netif);
static void  Enc28j60Callback_LinkChanged(struct netif *netif);
static struct netif *enc28j60  = 0;

void initLwIP(void)
{
	struct ip_addr ipaddr;
    struct ip_addr netmask;
    struct ip_addr gw;
    GPIO_InitTypeDef pin;
    MadBool ok;
    
    pin.GPIO_Mode  = GPIO_Mode_Out_PP;
	pin.GPIO_Pin   = GPIO_Pin_2;
	pin.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOA, &pin);
    GPIO_SetBits(GPIOA, pin.GPIO_Pin);
    
    ok = Enc28j60Port0Init();
    enc28j60 = madMemMalloc(sizeof(struct netif));
    if((MFALSE == ok) || (MNULL == enc28j60)) {
        Enc28j60Port0UnInit();
        madMemFreeNull(enc28j60);
        madThreadDelete(MAD_THREAD_SELF);
    }
    
	tcpip_init(0, 0);

#if LWIP_DHCP    
	IP4_ADDR(&gw, 0,0,0,0);
	IP4_ADDR(&ipaddr, 0,0,0,0);
	IP4_ADDR(&netmask, 0,0,0,0);
#else
	IP4_ADDR(&gw, 192,168,0,1);
	IP4_ADDR(&ipaddr, 192,168,0,56);
	IP4_ADDR(&netmask, 255,255,255,0);
#endif
    
	netif_add(enc28j60, &ipaddr, &netmask, &gw, EthENC28J60[0], ethernetif_init, tcpip_input);
	netif_set_default(enc28j60);
    netif_set_link_callback(enc28j60, Enc28j60Callback_LinkChanged);
#if LWIP_DHCP
    dhcp_start(enc28j60);
#else
	netif_set_up(enc28j60);
#endif
}

static void Enc28j60Callback_LinkChanged(struct netif *netif)
{
    if(netif->flags & NETIF_FLAG_LINK_UP) {
        GPIO_ResetBits(GPIOA, GPIO_Pin_2);
        /* Link up */
    } else {
        GPIO_SetBits(GPIOA, GPIO_Pin_2);
        /* Link down */
    }
}
