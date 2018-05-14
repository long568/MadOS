#include "uTcp.h"
#include "LoArm.h"
#include "LoStepMotor.h"
#include "LoDCMotor.h"
#include "UserConfig.h"

MadSemCB_t *LoArmCmd_Sig = 0;
LoArmCmd_t  LoArmCmdCur = {0, 0, 0, 0, 0};

LoStepMotor_t LoArm_Axis1;
LoStepMotor_t LoArm_Axis2;
LoDCMotor_t   LoArm_Axis3;
LoDCMotor_t   LoArm_Axis4;

#define LoArm_EN(x) do { \
    (x) ? GPIO_SetBits(LoArm_EN_G, LoArm_EN_P) : GPIO_ResetBits(LoArm_EN_G, LoArm_EN_P); \
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
static void LoArm_thread(MadVptr exData);

MadBool LoArm_Init(void)
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

    LoArm_Axis1.t = LoArm_AXIS1_TIM;
    LoArm_Axis1.g = LoArm_AXIS1_DIR_G;
    LoArm_Axis1.c = LoArm_AXIS1_CHL;
    LoArm_Axis1.p = LoArm_AXIS1_DIR_P;
    LoStepMotor_Init(&LoArm_Axis1);

    LoArm_Axis2.t = LoArm_AXIS2_TIM;
    LoArm_Axis2.g = LoArm_AXIS2_DIR_G;
    LoArm_Axis2.c = LoArm_AXIS2_CHL;
    LoArm_Axis2.p = LoArm_AXIS2_DIR_P;
    LoStepMotor_Init(&LoArm_Axis2);

    LoArm_Axis3.t  = LoArm_AXIS3_TIM;
    LoArm_Axis3.g  = LoArm_AXIS3_DIR_G;
    LoArm_Axis3.c  = LoArm_AXIS3_CHL;
    LoArm_Axis3.p1 = LoArm_AXIS3_DIR_P1;
    LoArm_Axis3.p1 = LoArm_AXIS3_DIR_P2;
    LoDCMotor_Init(&LoArm_Axis3);

    LoArm_Axis4.t  = LoArm_AXIS4_TIM;
    LoArm_Axis4.g  = LoArm_AXIS4_DIR_G;
    LoArm_Axis4.c  = LoArm_AXIS4_CHL;
    LoArm_Axis4.p1 = LoArm_AXIS4_DIR_P1;
    LoArm_Axis4.p1 = LoArm_AXIS4_DIR_P2;
    LoDCMotor_Init(&LoArm_Axis4);

    tcp_init();
    madThreadCreate(LoArm_thread, 0, 2048, THREAD_PRIO_MAD_ARM);
    return MTRUE;
}

void LoArm_thread(MadVptr exData)
{
    MadU8  ok;
    MadS8  a1;
    MadS8  a2;
    MadS8  a3;
    MadS8  a4;
    MadU32 key;

    (void) key;

    while(1) {
        ok = madSemWait(&LoArmCmd_Sig, 3000);
        if(MAD_ERR_OK == ok) {
            LoArmCmd_Get(a1, a2, a3, a4, key);
            LoStepMotor_Go(&LoArm_Axis1, a1);
            LoStepMotor_Go(&LoArm_Axis2, a2);
            LoDCMotor_Go  (&LoArm_Axis3, a3);
            LoDCMotor_Go  (&LoArm_Axis4, a4);
        } else {
            LoArmCmd_Clear();
            LoStepMotor_Go(&LoArm_Axis1, 0);
            LoStepMotor_Go(&LoArm_Axis2, 0);
            LoDCMotor_Go  (&LoArm_Axis3, 0);
            LoDCMotor_Go  (&LoArm_Axis4, 0);
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
