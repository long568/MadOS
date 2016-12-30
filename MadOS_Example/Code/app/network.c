#include "UserConfig.h"
#include "network.h"
#include "fatfs.h"
#include "cloud.h"

extern err_t ethernetif_init(struct netif *netif);

static void Enc28j60Callback_LinkChanged(struct netif *netif);
static void testTcpC(MadVptr exData);

static int          tcp_client = -1;
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

    madThreadCreate(testTcpC, 0, 1024, THREAD_PRIO_TEST_TCPC);
    madThreadCreate(cloud, 0, 1024, THREAD_PRIO_CLOUD);
}

static void Enc28j60Callback_LinkChanged(struct netif *netif)
{
    if(netif->flags & NETIF_FLAG_LINK_UP) {
        GPIO_ResetBits(GPIOA, GPIO_Pin_2);
        cloudLinkUp();
    } else {
        GPIO_SetBits(GPIOA, GPIO_Pin_2);
        cloudLinkDown();
        shutdown(tcp_client, SHUT_RDWR);
    }
}

static void testTcpC(MadVptr exData)
{
    int cnt;
    MadU8 *lwip_buffer;
    struct sockaddr_in local;
    struct sockaddr_in remote;
    
    (void)exData;
    
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
	remote.sin_addr.s_addr = inet_addr("192.168.1.100");
	remote.sin_port = htons(5685U);
    
    while(1) {
        do {
            tcp_client = socket(AF_INET, SOCK_STREAM, 0);
            if(0 > tcp_client) break;
            if(0 > bind(tcp_client, (struct sockaddr *)&local, sizeof(struct sockaddr))) break;
            if(0 > connect(tcp_client, (struct sockaddr *)&remote, sizeof(struct sockaddr))) break;
            
            do {
                FIL  *fil;
                UINT bw;
                UINT br;
                ip_addr_t addr_trg;
                char str_baidu[16];
                char str_wowstart[16];
                MadSize_t dns_size;
                MadU8 *sd_buff;
                dns_size = 0;
                fil = (FIL*)madMemMalloc(sizeof(FIL));
                if(0 == fil) break;
                if(FR_OK == f_open(fil, "dns", FA_CREATE_ALWAYS | FA_WRITE)) {
                    str_baidu[0] = 0;
                    str_wowstart[0] = 0;
                    netconn_gethostbyname("www.baidu.com", &addr_trg);
                    strcpy(str_baidu, (const char *)ipaddr_ntoa(&addr_trg));
                    netconn_gethostbyname("www.wowstart.org", &addr_trg);
                    strcpy(str_wowstart, (const char *)ipaddr_ntoa(&addr_trg));
                    f_write(fil, "www.baidu.com     ", strlen("www.baidu.com     "), &bw);
                    dns_size += bw;
                    f_write(fil, str_baidu, strlen(str_baidu), &bw);
                    dns_size += bw;
                    f_write(fil, "\r\n", strlen("\r\n"), &bw);
                    dns_size += bw;
                    f_write(fil, "www.wowstart.org  ", strlen("www.wowstart.org  "), &bw);
                    dns_size += bw;
                    f_write(fil, str_wowstart, strlen(str_wowstart), &bw);
                    dns_size += bw;
                    f_write(fil, "\r\n", strlen("\r\n"), &bw);
                    dns_size += bw;
                    f_close(fil);
                }
                sd_buff = madMemMalloc(strlen(TEST_STR) + dns_size);
                if(sd_buff) {
                    if(FR_OK == f_open(fil, "long", FA_OPEN_EXISTING | FA_READ)) {
                        f_read(fil, sd_buff, strlen(TEST_STR), &br);
                        f_close(fil);
                    }
                    if(FR_OK == f_open(fil, "dns", FA_OPEN_EXISTING | FA_READ)) {
                        f_read(fil, sd_buff + strlen(TEST_STR), dns_size, &br);
                        f_close(fil);
                    }
                    write(tcp_client, sd_buff, strlen(TEST_STR) + dns_size);
                    madMemFree(sd_buff);
                }
                madMemFree(fil);
            } while(0);
            
            while(1) {
                cnt = read(tcp_client, lwip_buffer, LWIP_BUFFER_SIZE);
                if(0 >= cnt) {
                    break;
                } else {
                    write(tcp_client, lwip_buffer, cnt);
                }
            }
        } while(0);
        close(tcp_client);
        tcp_client = -1;
        madTimeDly(1000);
    }
}
