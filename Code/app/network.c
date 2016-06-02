#include "network.h"

#define LWIP_BUFFER_SIZE 128

extern err_t ethernetif_init(struct netif *netif);

MadStatic FATFS *fat_MicroSD;
MadStatic int tcp_client;
MadStatic struct netif *enc28j60;

static void Enc28j60Callback_LinkChanged(struct netif *netif)
{
    if(netif->flags & NETIF_FLAG_LINK_UP) {
        GPIO_ResetBits(GPIOA, GPIO_Pin_2);
    } else {
        GPIO_SetBits(GPIOA, GPIO_Pin_2);
        shutdown(tcp_client, SHUT_RDWR);
    }
}

static void initMicroSD(void)
{
    FIL fil;
    UINT bw;
    fat_MicroSD = madMemMalloc(sizeof(FATFS));
    if(fat_MicroSD) {
        if(FR_OK == f_mount(fat_MicroSD, "", 0)) {
            if(FR_OK == f_open(&fil, "long", FA_CREATE_ALWAYS | FA_WRITE)) {
                if(FR_OK == f_write(&fil, TEST_STR, sizeof(TEST_STR), &bw)) {
                    GPIO_ResetBits(GPIOA, GPIO_Pin_3);
                }
                f_close(&fil);
            }
        } else {
            madMemSafeFree(fat_MicroSD);
        }
    }
}

void initLwIP(void)
{
	struct ip_addr ipaddr, netmask, gw;
    GPIO_InitTypeDef pin;
    
    pin.GPIO_Mode  = GPIO_Mode_Out_PP;
	pin.GPIO_Pin   = GPIO_Pin_2 | GPIO_Pin_3;
	pin.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOA, &pin);
    GPIO_SetBits(GPIOA, pin.GPIO_Pin);
    
    enc28j60 = madMemMalloc(sizeof(struct netif));
    EthENC28J60 = madMemMalloc(sizeof(DevENC28J60));
    if((!EthENC28J60) || (!enc28j60)) {
        while(1) {
            __asm volatile ( "NOP" );
        }
    }
    
    initMicroSD();
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
    
	netif_add(enc28j60, &ipaddr, &netmask, &gw, EthENC28J60, ethernetif_init, tcpip_input);
	netif_set_default(enc28j60);
    netif_set_link_callback(enc28j60, Enc28j60Callback_LinkChanged);
#if LWIP_DHCP
    dhcp_start(enc28j60);
#else
	netif_set_up(enc28j60);
#endif
}

void testTcpSocket(MadVptr exData)
{
    int cnt;
    struct sockaddr_in local, remote;
    MadU8 *sd_buff;
    MadU8 *lwip_buffer;
//    MadU8 lwip_buffer[LWIP_BUFFER_SIZE];
    
    sd_buff = 0;
    
    lwip_buffer = madMemMalloc(LWIP_BUFFER_SIZE);
    if(!lwip_buffer) {
        madThreadDelete(MAD_THREAD_SELF);
    }
    
    local.sin_len = sizeof(struct sockaddr_in);
	local.sin_family = AF_INET;
	local.sin_addr.s_addr = INADDR_ANY;
	local.sin_port = htons(8888U);
	
	remote.sin_len = sizeof(struct sockaddr_in);
	remote.sin_family = AF_INET;
//	remote.sin_addr.s_addr = inet_addr("121.42.9.78");
	remote.sin_addr.s_addr = inet_addr("192.168.0.100");
	remote.sin_port = htons(5685U);
    
    while(1) {
        do {
            FIL fil;
            UINT br;
            if(FR_OK == f_open(&fil, "long", FA_OPEN_EXISTING | FA_READ)) {
                sd_buff = madMemMalloc(sizeof(TEST_STR));
                if(sd_buff) {
                    if(FR_OK != f_read(&fil, sd_buff, sizeof(TEST_STR), &br)) {
                        madMemFreeNull(sd_buff);
                    }
                }
                f_close(&fil);
            }
        } while(0);
        
        do {
            tcp_client = socket(AF_INET, SOCK_STREAM, 0);
            if(0 > tcp_client) break;
            if(0 > bind(tcp_client, (struct sockaddr *)&local, sizeof(struct sockaddr))) break;
            if(0 > connect(tcp_client, (struct sockaddr *)&remote, sizeof(struct sockaddr))) break;
            if(sd_buff) {
                write(tcp_client, sd_buff, sizeof(TEST_STR));
                madMemFreeNull(sd_buff);
            }
            
            while(1) {
                cnt = read(tcp_client, lwip_buffer, LWIP_BUFFER_SIZE);
                if(0 >= cnt) {
                    break;
                } else {
                    write(tcp_client, lwip_buffer, cnt);
                }
            }
        } while(0);
        
        if(sd_buff)
            madMemFreeNull(sd_buff);
        close(tcp_client);
        madTimeDly(1000);
    }
}
