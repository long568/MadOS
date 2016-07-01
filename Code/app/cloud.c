#include "cloud.h"

#define setBlink(t)      MAD_OPT_IN_CRITICAL(time = t;)
#define getBlink(t)      MAD_OPT_IN_CRITICAL(t = *(MadTim_t*)time;)
#define setLink()        MAD_OPT_IN_CRITICAL(cloud_linked = MTRUE;)
#define resetLink()      MAD_OPT_IN_CRITICAL(cloud_linked = MFALSE;)
#define checkLink()      MAD_OPT_IN_CRITICAL(if(MFALSE == cloud_linked) { madThreadPend(MAD_THREAD_SELF); })
#define setRunning()     MAD_OPT_IN_CRITICAL(cloud_running = MTRUE;)
#define getRunning(x)    MAD_OPT_IN_CRITICAL(x = cloud_running;)

static void blinkLed(MadVptr time);
#if CLOUD_USE_HEARTBEAT
static void heartBeat(MadVptr data);
#endif

static int     cloud_client  = -1;
static MadBool cloud_linked  = MFALSE;
static MadBool cloud_running = MFALSE;

const char beatData[] = {0, 1, 2};
const char idWorker[] = {CMD_HEAD_HEAD, CMD_HEAD_HEAD, CMD_HEAD_ACT_ID, CMD_HEAD_ID_WORKER, 0, 0, CMD_HEAD_TAIL, CMD_HEAD_TAIL};

void cloudLinkUp(void)
{
    MadBool running;
    setLink();
    getRunning(running);
    if(MTRUE == running)
        madThreadResume(THREAD_PRIO_CLOUD);
}

void cloudLinkDown(void)
{
    MadBool running;
    resetLink();
    getRunning(running);
    if(MTRUE == running)
        shutdown(cloud_client, SHUT_RDWR);
}

void cloud(MadVptr exData)
{
    FIL       *fil;
    UINT      bw;
    MadU8     *buffer;
    MadUint   cnt;
    MadTim_t  time;
    ip_addr_t ip;
    struct sockaddr_in local;
    struct sockaddr_in remote;
    
    (void)exData;
    
    do {
        GPIO_InitTypeDef pin;
        pin.GPIO_Mode  = GPIO_Mode_Out_PP;
        pin.GPIO_Pin   = GPIO_Pin_3;
        pin.GPIO_Speed = GPIO_Speed_2MHz;
        GPIO_Init(GPIOA, &pin);
        GPIO_SetBits(GPIOA, pin.GPIO_Pin);
    } while(0);
    
    fil = (FIL*)madMemMalloc(sizeof(FIL));
    buffer = madMemMalloc(CLOUD_BUFFER_SIZE);
    if((0 == buffer) || (0 == fil)) {
        madMemFree(fil);
        madMemFree(buffer);
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
#if CLOUD_USE_HEARTBEAT
    madThreadCreate(heartBeat, 0, 256, THREAD_PRIO_CLOUD_HEARTBEAT);
    madThreadPend(THREAD_PRIO_CLOUD_HEARTBEAT);
#endif
    
    setRunning();
    while(1) {
        do {
            if(ERR_OK != netconn_gethostbyname("www.wowstart.org", &ip)) {
                break;
            }
            remote.sin_addr.s_addr = ip.addr;
            setBlink(500);
            cloud_client = socket(AF_INET, SOCK_STREAM, 0);
            if(0 > cloud_client) break;
            if(0 > bind(cloud_client, (struct sockaddr *)&local, sizeof(struct sockaddr))) break;
            if(0 > connect(cloud_client, (struct sockaddr *)&remote, sizeof(struct sockaddr))) break;
            if(FR_OK != f_open(fil, "cloud", FA_OPEN_ALWAYS | FA_WRITE)) break;
            write(cloud_client, idWorker, CMD_HEAD_LEN);
            madThreadPend(THREAD_PRIO_CLOUD_BLINK);
#if CLOUD_USE_HEARTBEAT
            madThreadResume(THREAD_PRIO_CLOUD_HEARTBEAT);
#endif
            GPIO_ResetBits(GPIOA, GPIO_Pin_3);
            while(1) {
                cnt = read(cloud_client, buffer, CLOUD_BUFFER_SIZE);
                if(0 >= cnt) {
                    f_close(fil);
#if CLOUD_USE_HEARTBEAT
                    madThreadPend(THREAD_PRIO_CLOUD_HEARTBEAT);
#endif
                    madThreadResume(THREAD_PRIO_CLOUD_BLINK);
                    break;
                } else {
                    static MadBool led = MTRUE;
                    led = !led;
                    if(led)
                        GPIO_ResetBits(GPIOA, GPIO_Pin_3);
                    else
                        GPIO_SetBits(GPIOA, GPIO_Pin_3);
                    if(FR_OK != f_write(fil, buffer, cnt, &bw)) {
                        f_close(fil);
                        break;
                    }
                }
            }
        } while(0);
        setBlink(100);
        close(cloud_client);
        cloud_client = -1;
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

#if CLOUD_USE_HEARTBEAT
static void heartBeat(MadVptr data)
{
    while(1) {
        madTimeDly(30 * 1000);
        write(cloud_client, beatData, 3);
    }
}
#endif
