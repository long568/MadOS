#include "MadArm.h"
#include "uTcp.h"
#include "drv8825.h"
#include "uln2803a.h"

#define TIME_LINE_BASE  (72000000 / 1000000) // 1MHz
#define TIME_LINE_SCALE (TIME_LINE_BASE - 1)
#define TIME_LINE_PWM   (100 - 1)            // 10KHz

#define MotorCall(m, f, ...) (motor_##m)->f((motor_##m), ##__VA_ARGS__)

#define SET_CMD() \
do {                            \
    MadCpsr_t cpsr;             \
    madEnterCritical(cpsr);     \
    ArmCmd.left.dir   = cmd[0]; \
    ArmCmd.left.spd   = cmd[1]; \
    ArmCmd.right.dir  = cmd[2]; \
    ArmCmd.right.spd  = cmd[3]; \
    ArmCmd.top.dir    = cmd[4]; \
    ArmCmd.top.spd    = cmd[5]; \
    ArmCmd.bottom.dir = cmd[6]; \
    ArmCmd.bottom.spd = cmd[7]; \
    madExitCritical(cpsr);      \
} while(0)

#define CLR_CMD() \
do {                        \
    MadCpsr_t cpsr;         \
    madEnterCritical(cpsr); \
    ArmCmd.left.dir   = 0;  \
    ArmCmd.left.spd   = 0;  \
    ArmCmd.right.dir  = 0;  \
    ArmCmd.right.spd  = 0;  \
    ArmCmd.top.dir    = 0;  \
    ArmCmd.top.spd    = 0;  \
    ArmCmd.bottom.dir = 0;  \
    ArmCmd.bottom.spd = 0;  \
    madExitCritical(cpsr);  \
} while(0)

#define UPDATE_CMD(x) \
do {                                 \
    MadCpsr_t cpsr;                  \
    madEnterCritical(cpsr);          \
    motor_##x->dir_n = ArmCmd.x.dir; \
    motor_##x->spd_n = ArmCmd.x.spd; \
    madExitCritical(cpsr);           \
} while(0)

static MadSemCB_t *tim_sig;

static StmPIN motor_en;
static Motor *motor_left;
static Motor *motor_right;
static Motor *motor_bottom;
static Motor *motor_top;

static MotionCmd ArmCmd = {
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
};

static void app_main(MadVptr exData);
static void tim_init(void);
static void tim_startup(void);

static const char tcp_ack[] = "A";

static uIP_App     client;
static struct pt   pt_tcp;
static uIP_TcpConn *conn_tcp;

static void tcp_init(void);
static void tcp_link_changed(MadVptr ep);
static void tcp_start_up(void);
static void tcp_shut_down(void);
static void tcp_appcall(MadVptr ep);

/*
 ************************************************************
 *
 * Motion
 *
 ************************************************************
 */
void Init_MadArm(void)
{
    tim_init();
    tcp_init();
    
    StmPIN_SetIO(&motor_en, GPIOC, GPIO_Pin_13);
    StmPIN_DefInitOPP(&motor_en);
    StmPIN_SetLow(&motor_en);
    
    motor_left   = (Motor*)madMemMalloc(sizeof(Motor));
    motor_right  = (Motor*)madMemMalloc(sizeof(Motor));
    StmPIN_SetIO(&motor_left->io.a,  GPIOE, GPIO_Pin_3);
    StmPIN_SetIO(&motor_left->io.b,  GPIOE, GPIO_Pin_2);
    StmPIN_SetIO(&motor_right->io.a, GPIOE, GPIO_Pin_5);
    StmPIN_SetIO(&motor_right->io.b, GPIOE, GPIO_Pin_4);
    Motor_DRV8825(motor_left);
    Motor_DRV8825(motor_right);
    
    motor_top    = (Motor*)madMemMalloc(sizeof(Motor));
    motor_bottom = (Motor*)madMemMalloc(sizeof(Motor));
    StmPIN_SetIO(&motor_top->io.a,    GPIOE, GPIO_Pin_8);
    StmPIN_SetIO(&motor_top->io.b,    GPIOE, GPIO_Pin_9);
    StmPIN_SetIO(&motor_top->io.c,    GPIOE, GPIO_Pin_10);
    StmPIN_SetIO(&motor_top->io.d,    GPIOE, GPIO_Pin_11);
    StmPIN_SetIO(&motor_bottom->io.a, GPIOE, GPIO_Pin_12);
    StmPIN_SetIO(&motor_bottom->io.b, GPIOE, GPIO_Pin_13);
    StmPIN_SetIO(&motor_bottom->io.c, GPIOE, GPIO_Pin_14);
    StmPIN_SetIO(&motor_bottom->io.d, GPIOE, GPIO_Pin_15);
    Motor_ULN2803(motor_top);
    Motor_ULN2803(motor_bottom);
    
    madThreadCreate(app_main, 0, 2048, THREAD_PRIO_MAD_ARM);
}

static void app_main(MadVptr exData)
{
    MadBool flag;
    (void)exData;
    
    UPDATE_CMD(left);
    UPDATE_CMD(right);
    UPDATE_CMD(top);
    UPDATE_CMD(bottom);
    
    tim_startup();
    StmPIN_SetHigh(&motor_en);
    
    while(1) {
        if(MotorCall(left,   step)) UPDATE_CMD(left);
        if(MotorCall(right,  step)) UPDATE_CMD(right);
        if(MotorCall(top,    step)) UPDATE_CMD(top);
        if(MotorCall(bottom, step)) UPDATE_CMD(bottom);
        if(flag) { // Only for testing ...
            GPIO_ResetBits(GPIOE, GPIO_Pin_0);
        } else {
            GPIO_SetBits  (GPIOE, GPIO_Pin_0);
        }
        flag = !flag;
        madSemWait(&tim_sig, 0);
    }
}

static void tim_init(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);
    
    do { // Only for testing ...
        GPIO_InitTypeDef pin;
        pin.GPIO_Mode  = GPIO_Mode_Out_PP;
        pin.GPIO_Speed = GPIO_Speed_50MHz;
        pin.GPIO_Pin   = GPIO_Pin_0;
        GPIO_Init(GPIOE, &pin);
    } while(0);
    
    do {
        NVIC_InitTypeDef NVIC_InitStructure;
        NVIC_InitStructure.NVIC_IRQChannel    = TIM6_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = ISR_PRIO_TIM6;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 0;
        NVIC_Init(&NVIC_InitStructure);
    } while(0);
    
    tim_sig = madSemCreate(1);
    if(tim_sig) {
        madSemWait(&tim_sig, 0);
    }
}

static void tim_startup(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_DeInit(TIM6);
    TIM_TimeBaseStructure.TIM_Prescaler         = TIME_LINE_SCALE;
    TIM_TimeBaseStructure.TIM_CounterMode       = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_Period            = TIME_LINE_PWM;
    TIM_TimeBaseStructure.TIM_ClockDivision     = 0;
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure);
    TIM_UpdateRequestConfig(TIM6, TIM_UpdateSource_Regular);
//    TIM_ClearFlag(TIM6, TIM_FLAG_Update);
    TIM_ITConfig(TIM6, TIM_IT_Update, ENABLE);
    TIM_Cmd(TIM6, ENABLE);
}

void TIM6_IRQHandler(void)
{
    if(TIM_GetITStatus(TIM6, TIM_IT_Update) != RESET) {
        madSemRelease(&tim_sig);
        TIM_ClearITPendingBit(TIM6, TIM_IT_Update);
    }
}

/*
 ************************************************************
 *
 * Tcp
 *
 ************************************************************
 */
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
    CLR_CMD();
}

/*********************************************
 * Tcp Appcall
 *********************************************/
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
    MA_SERVER_IP(ipaddr);
    uip_connect(&ipaddr, MA_SERVER_PORT());
    PT_WAIT_UNTIL(pt, tcp_is_connected());
    CHECK_IF_RESTART();

    do {
        PT_YIELD(pt);
        CHECK_IF_RESTART();
        if(uip_newdata()) {
            unsigned char *cmd = uip_appdata;
            SET_CMD();
            uip_send(tcp_ack, 1);
        }
    } while(1);
    
    PT_END(pt);
}

void tcp_appcall(MadVptr ep) { tcp_pt(ep); }
