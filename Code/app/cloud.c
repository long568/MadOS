#include "cloud.h"

#define setBlink(t)  MAD_OPT_IN_CRITICAL(time = t;)
#define getBlink(t)  MAD_OPT_IN_CRITICAL(t = *(MadTim_t*)time;)
#define setLink()    MAD_OPT_IN_CRITICAL(tcp_linked = MTRUE;)
#define resetLink()  MAD_OPT_IN_CRITICAL(tcp_linked = MFALSE;)
#define checkLink()  MAD_OPT_IN_CRITICAL(if(MFALSE == tcp_linked) { madThreadPend(MAD_THREAD_SELF); })

static void blinkLed(MadVptr time);
static void heartBeat(MadVptr data);

static int     tcp_client;
static MadBool tcp_linked;

const char beatData[] = {0, 1, 2};
const char idWorker[] = {CMD_HEAD_HEAD, CMD_HEAD_HEAD, CMD_HEAD_ACT_ID, CMD_HEAD_ID_WORKER, 0, 0, CMD_HEAD_TAIL, CMD_HEAD_TAIL};

void cloudLinkUp(void)
{
    setLink();
    madThreadResume(THREAD_PRIO_CLOUD);
}

void cloudLinkDown(void)
{
    resetLink();
    shutdown(tcp_client, SHUT_RDWR);
}

void cloud(MadVptr exData)
{
    FIL       fil;
    UINT      bw;
    UINT      total;
    MadU8     *buffer;
    MadUint   cnt;
    MadTim_t  time;
    ip_addr_t ip;
    struct sockaddr_in local;
    struct sockaddr_in remote;
    
    do {
        GPIO_InitTypeDef pin;
        pin.GPIO_Mode  = GPIO_Mode_Out_PP;
        pin.GPIO_Pin   = GPIO_Pin_3;
        pin.GPIO_Speed = GPIO_Speed_2MHz;
        GPIO_Init(GPIOA, &pin);
        GPIO_SetBits(GPIOA, pin.GPIO_Pin);
    } while(0);
    
    buffer = madMemMalloc(CLOUD_BUFFER_SIZE);
    if(!buffer) {
        madThreadDelete(MAD_THREAD_SELF);
    }
    
    local.sin_len = sizeof(struct sockaddr_in);
	local.sin_family = AF_INET;
	local.sin_addr.s_addr = INADDR_ANY;
	local.sin_port = 0; //htons(6666U);
    
    remote.sin_len = sizeof(struct sockaddr_in);
    remote.sin_family = AF_INET;
    remote.sin_addr.s_addr = 0;
    remote.sin_port = htons(5685U);
    
    time = 100;
    madThreadCreate(blinkLed, &time, 128, THREAD_PRIO_CLOUD_BLINK);
    madThreadCreate(heartBeat, 0, 256, THREAD_PRIO_CLOUD_HEARTBEAT);
    madThreadPend(THREAD_PRIO_CLOUD_HEARTBEAT);
    
    while(1) {
        do {
            if(ERR_OK != netconn_gethostbyname("www.wowstart.org", &ip)) {
                break;
            }
            remote.sin_addr.s_addr = ip.addr;
            setBlink(500);
            tcp_client = socket(AF_INET, SOCK_STREAM, 0);
            if(0 > tcp_client) break;
            if(0 > bind(tcp_client, (struct sockaddr *)&local, sizeof(struct sockaddr))) break;
            if(0 > connect(tcp_client, (struct sockaddr *)&remote, sizeof(struct sockaddr))) break;
            if(FR_OK != f_open(&fil, "cloud", FA_OPEN_ALWAYS | FA_WRITE)) break;
            write(tcp_client, idWorker, CMD_HEAD_LEN);
            madThreadPend(THREAD_PRIO_CLOUD_BLINK);
            madThreadResume(THREAD_PRIO_CLOUD_HEARTBEAT);
            GPIO_ResetBits(GPIOA, GPIO_Pin_3);
            total = 0;
            while(1) {
                cnt = read(tcp_client, buffer, CLOUD_BUFFER_SIZE);
                if(0 >= cnt) {
                    f_close(&fil);
                    madThreadPend(THREAD_PRIO_CLOUD_HEARTBEAT);
                    madThreadResume(THREAD_PRIO_CLOUD_BLINK);
                    break;
                } else {
                    static MadBool led = MTRUE;
                    led = !led;
                    if(led)
                        GPIO_ResetBits(GPIOA, GPIO_Pin_3);
                    else
                        GPIO_SetBits(GPIOA, GPIO_Pin_3);
                    total += cnt;
                    if(total > 512) {
                        if(FR_OK != f_sync(&fil)) {
                            f_close(&fil);
                            break;
                        }
                        total = cnt;
                    }
                    if(FR_OK != f_write(&fil, buffer, cnt, &bw)) {
                        f_close(&fil);
                        break;
                    }
                }
            }
        } while(0);
        setBlink(100);
        close(tcp_client);
        checkLink();
        madTimeDly(6000);
    }
}

static void blinkLed(MadVptr time)
{
    static MadBool led = MFALSE;
    MadTim_t t;
    while(1) {
        getBlink(t);
        madTimeDly(t);
        led = !led;
        if(led)
            GPIO_ResetBits(GPIOA, GPIO_Pin_3);
        else
            GPIO_SetBits(GPIOA, GPIO_Pin_3);
    }
}

static void heartBeat(MadVptr data)
{
    while(1) {
        madTimeDly(30 * 1000);
        write(tcp_client, beatData, 3);
    }
}
