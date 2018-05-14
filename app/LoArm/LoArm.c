#include "LoArm.h"
#include "uTcp.h"

StmPIN LoArm_En;
MadSemCB_t *LoArmCmd_Sig = 0;
LoArmCmd_t LoArmCmdOld = {0, 0, 0, 0, 0};
LoArmCmd_t LoArmCmdCur = {0, 0, 0, 0, 0};

#define LoArm_EN(x) do { \
    (x) ? GPIO_SetBits(LoArm_EN_G, LoArm_EN_P) : GPIO_ResetBits(LoArm_EN_G, LoArm_EN_P); \
} while(0)

#define LoArm_Axis1_Dir(x) do { \
    (x) ? GPIO_SetBits(LoArm_AXIS1_DIR_G, LoArm_AXIS1_DIR_P) : GPIO_ResetBits(LoArm_AXIS1_DIR_G, LoArm_AXIS1_DIR_P); \
} while(0)

#define LoArm_Axis2_Dir(x) do { \
    (x) ? GPIO_SetBits(LoArm_AXIS2_DIR_G, LoArm_AXIS2_DIR_P) : GPIO_ResetBits(LoArm_AXIS2_DIR_G, LoArm_AXIS2_DIR_P); \
} while(0)

#define LoArm_Axis3_Dir(x) do { \
    GPIO_ResetBits(LoArm_AXIS3_DIR_G, LoArm_AXIS3_DIR_P1 | LoArm_AXIS3_DIR_P2); \
    (x) ? GPIO_SetBits(LoArm_AXIS3_DIR_G, LoArm_AXIS3_DIR_P1) : GPIO_SetBits(LoArm_AXIS3_DIR_G, LoArm_AXIS3_DIR_P2); \
} while(0)

#define LoArm_Axis3_Lock(x) do { \
    (x) ? GPIO_ResetBits(LoArm_AXIS3_DIR_G, LoArm_AXIS3_DIR_P1 | LoArm_AXIS3_DIR_P2) : \
          GPIO_SetBits  (LoArm_AXIS3_DIR_G, LoArm_AXIS3_DIR_P1 | LoArm_AXIS3_DIR_P2);  \
} while(0)

#define LoArm_Axis4_Dir(x) do { \
    GPIO_ResetBits(LoArm_AXIS4_DIR_G, LoArm_AXIS4_DIR_P1 | LoArm_AXIS4_DIR_P2); \
    (x) ? GPIO_SetBits(LoArm_AXIS4_DIR_G, LoArm_AXIS4_DIR_P1) : GPIO_SetBits(LoArm_AXIS4_DIR_G, LoArm_AXIS4_DIR_P2); \
} while(0)

#define LoArm_Axis4_Lock(x) do { \
    (x) ? GPIO_ResetBits(LoArm_AXIS4_DIR_G, LoArm_AXIS4_DIR_P1 | LoArm_AXIS4_DIR_P2) : \
          GPIO_SetBits  (LoArm_AXIS4_DIR_G, LoArm_AXIS4_DIR_P1 | LoArm_AXIS4_DIR_P2);  \
} while(0)

#if 0
    MadSemCB_t *LoArmCmd_Locker = 0;
#   define LoArmCmd_Lock()   do { madSemWait(&LoArmCmd_Locker, 0);
#   define LoArmCmd_Unlock() madSemRelease(&LoArmCmd_Locker); } while(0)
#else
#   define LoArmCmd_Lock()   do { MadCpsr_t cpsr; madEnterCritical(cpsr);
#   define LoArmCmd_Unlock() madExitCritical(cpsr); } while(0)
#endif

#define LoArmCmd_Set() \
    LoArmCmd_Lock()             \
    LoArmCmdCur.axis1 = cmd[0]; \
    LoArmCmdCur.axis2 = cmd[1]; \
    LoArmCmdCur.axis3 = cmd[2]; \
    LoArmCmdCur.axis4 = cmd[3]; \
    LoArmCmdCur.key   = cmd[4]; \
    LoArmCmd_Unlock()

#define LoArmCmd_Get(a1, a2, a3, a4, key) \
    LoArmCmd_Lock()          \
    a1  = LoArmCmdCur.axis1; \
    a2  = LoArmCmdCur.axis2; \
    a3  = LoArmCmdCur.axis3; \
    a4  = LoArmCmdCur.axis4; \
    key = LoArmCmdCur.key;   \
    LoArmCmd_Unlock()

#define LoArmCmd_Clear() \
    LoArmCmd_Lock()        \
    LoArmCmdCur.axis1 = 0; \
    LoArmCmdCur.axis2 = 0; \
    LoArmCmdCur.axis3 = 0; \
    LoArmCmdCur.axis4 = 0; \
    LoArmCmdCur.key   = 0; \
    LoArmCmd_Unlock()

static void tcp_init(void);
static void tim_init(TIM_TypeDef* TIMx, uint16_t Chl);
static void tim_startup(TIM_TypeDef* TIMx, uint16_t Chl, MadBool f);
static void lo_arm_thread(MadVptr exData);

MadBool Init_LoArm(void)
{
    GPIO_InitTypeDef pin;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);

    GPIO_PinRemapConfig(GPIO_PartialRemap2_TIM2, ENABLE);
    GPIO_PinRemapConfig(GPIO_FullRemap_TIM3,     ENABLE);
    GPIO_PinRemapConfig(GPIO_Remap_TIM4,         ENABLE);

    pin.GPIO_Speed = GPIO_Speed_50MHz;
    pin.GPIO_Mode  = GPIO_Mode_AF_PP;
    pin.GPIO_Pin   = LoArm_AXIS1_PWM_P;
    GPIO_Init(LoArm_AXIS1_PWM_G, &pin);
    pin.GPIO_Pin   = LoArm_AXIS2_PWM_P;
    GPIO_Init(LoArm_AXIS2_PWM_G, &pin);
    pin.GPIO_Pin   = LoArm_AXIS3_PWM_P;
    GPIO_Init(LoArm_AXIS3_PWM_G, &pin);
    pin.GPIO_Pin   = LoArm_AXIS4_PWM_P;
    GPIO_Init(LoArm_AXIS4_PWM_G, &pin);
    
    pin.GPIO_Mode  = GPIO_Mode_Out_PP;
    pin.GPIO_Pin   = LoArm_EN_P;
    GPIO_Init(LoArm_EN_G, &pin);
    pin.GPIO_Pin   = LoArm_AXIS1_DIR_P;
    GPIO_Init(LoArm_AXIS1_DIR_G, &pin);
    pin.GPIO_Pin   = LoArm_AXIS2_DIR_P;
    GPIO_Init(LoArm_AXIS2_DIR_G, &pin);
    pin.GPIO_Pin   = LoArm_AXIS3_DIR_P1;
    GPIO_Init(LoArm_AXIS3_DIR_G, &pin);
    pin.GPIO_Pin   = LoArm_AXIS3_DIR_P2;
    GPIO_Init(LoArm_AXIS3_DIR_G, &pin);
    pin.GPIO_Pin   = LoArm_AXIS4_DIR_P1;
    GPIO_Init(LoArm_AXIS4_DIR_G, &pin);
    pin.GPIO_Pin   = LoArm_AXIS4_DIR_P2;
    GPIO_Init(LoArm_AXIS4_DIR_G, &pin);

    LoArm_Axis3_Lock(1);
    LoArm_Axis4_Lock(1);
    LoArm_EN(1);

    tim_init(LoArm_AXIS1_TIM, LoArm_AXIS1_CHL);
    tim_init(LoArm_AXIS2_TIM, LoArm_AXIS2_CHL);
    tim_init(LoArm_AXIS3_TIM, LoArm_AXIS3_CHL);
    tim_init(LoArm_AXIS4_TIM, LoArm_AXIS4_CHL);

    madThreadCreate(madSysRunning, 0, 2048, THREAD_PRIO_MAD_ARM);
    return MTRUE;
}

void tim_init(TIM_TypeDef* TIMx, uint16_t Chl)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef       TIM_OCStructure;

    TIM_DeInit(TIMx);
    TIM_TimeBaseStructure.TIM_Prescaler         = LoArm_TIME_SCALE;
    TIM_TimeBaseStructure.TIM_CounterMode       = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_Period            = LoArm_TIME_PWM;
    TIM_TimeBaseStructure.TIM_ClockDivision     = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIMx, &TIM_TimeBaseStructure);
    // TIM_UpdateRequestConfig(TIMx, TIM_UpdateSource_Regular);

    TIM_OCStructure.TIM_OCMode       = TIM_OCMode_PWM1;
    TIM_OCStructure.TIM_OutputState  = TIM_OutputState_Disable; //TIM_OutputState_Enable;
    TIM_OCStructure.TIM_OutputNState = TIM_OutputNState_Disable;
    TIM_OCStructure.TIM_Pulse        = LoArm_TIME_PWM / 2;
    TIM_OCStructure.TIM_OCPolarity   = TIM_OCPolarity_High;
    TIM_OCStructure.TIM_OCNPolarity  = TIM_OCNPolarity_High;
    TIM_OCStructure.TIM_OCIdleState  = TIM_OCIdleState_Set;
    TIM_OCStructure.TIM_OCNIdleState = TIM_OCNIdleState_Reset;

    switch (Chl) {
        case TIM_Channel_1: TIM_OC1Init(TIMx, &TIM_OCStructure); break;
        case TIM_Channel_2: TIM_OC2Init(TIMx, &TIM_OCStructure); break;
        case TIM_Channel_3: TIM_OC3Init(TIMx, &TIM_OCStructure); break;
        case TIM_Channel_4: TIM_OC4Init(TIMx, &TIM_OCStructure); break;
        default: break;
    }

    TIM_Cmd(TIMx, ENABLE);
    // TIM_CCxCmd(TIMx, Chl, TIM_CCx_Enable);
}

inline void tim_startup(TIM_TypeDef* TIMx, uint16_t Chl, MadBool f) {
    (f) ? TIM_CCxCmd(TIMx, Chl, TIM_CCx_Enable) : TIM_CCxCmd(TIMx, Chl, TIM_CCx_Disable);
}

void lo_arm_thread(MadVptr exData)
{
    MadU8  ok;
    MadS8  a1;
    MadS8  a2;
    MadS8  a3;
    MadS8  a4;
    MadU32 key;

    while(1) {
        ok = madSemWait(&LoArmCmd_Sig, 3000);
        if(MAD_ERR_OK == ok) {
            LoArmCmd_Get(a1, a2, a3, a4, key);

            // if(a1 != LoArmCmdOld.axis1) {
            //     LoArmCmdOld.axis1 = a1;
            //     if(a1 > 0) {
            //         LoArm_Axis1_Dir(1);
            //         TIM_CCxCmd(TIMx, Chl, TIM_CCx_Enable);
            //     } else if (a1 < 0) {
            //         LoArm_Axis1_Dir(0);
            //         TIM_CCxCmd(TIMx, Chl, TIM_CCx_Enable);
            //     } else {
            //         TIM_CCxCmd(TIMx, Chl, TIM_CCx_Disable);
            //     }
            // }
        } else {
            LoArmCmd_Clear();
        }
    }
}

/*
 ************************************************************
 *
 * Tcp
 *
 ************************************************************
 */
static const char  tcp_ack[] = "A";

static uIP_App     client;
static struct pt   pt_tcp;
static uIP_TcpConn *conn_tcp;

static void tcp_link_changed(MadVptr ep);
static void tcp_start_up(void);
static void tcp_shut_down(void);
static void tcp_appcall(MadVptr ep);

void tcp_init(void)
{
    client.link_changed = tcp_link_changed;
    client.resolv_found = MNULL;
    uIP_AppRegister(&client);
}

void tcp_link_changed(MadVptr ep)
{
    MadU32 linked = (MadU32)ep;
    if(uIP_LINKED_OFF == linked) {
        tcp_shut_down();
    } else {
        tcp_start_up();
    }
}

void tcp_start_up(void)
{
    conn_tcp = uip_new();
    if(conn_tcp) {
        uIP_SetTcpConn(conn_tcp, tcp_appcall);
        PT_INIT(&pt_tcp);
    }
}

void tcp_shut_down(void)
{
    uIP_SetTcpConn(conn_tcp, MNULL); 
    uip_remove(conn_tcp);
}

#define CHECK_IF_RESTART() \
do {                                        \
    if(TCP_FLAG_ERR == tcp_is_err()) {      \
        uIP_SetTcpConn(conn_tcp, MNULL);    \
        tcp_start_up();                     \
        return PT_EXITED;                   \
    }                                       \
} while(0)

static PT_THREAD(tcp_pt(MadVptr ep))
{
    static struct pt *pt = &pt_tcp;
    
    PT_BEGIN(pt);

    uip_ipaddr_t ipaddr;
    LoArm_SERVER_IP(ipaddr);
    uip_connect(&ipaddr, LoArm_SERVER_PORT());
    PT_WAIT_UNTIL(pt, tcp_is_connected());
    CHECK_IF_RESTART();

    do {
        PT_YIELD(pt);
        CHECK_IF_RESTART();
        if(uip_newdata()) {
            unsigned char *cmd = uip_appdata;
            LoArmCmd_Set();
            uip_send(tcp_ack, 1);
            madSemRelease(&LoArmCmd_Sig);
        }
    } while(1);
    
    PT_END(pt);
}

void tcp_appcall(MadVptr ep) { tcp_pt(ep); }
