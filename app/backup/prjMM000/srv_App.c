#include "MadOS.h"
#include "pt.h"
#include "timer.h"
#include "CfgUser.h"
#include "srv_App.h"
#include "LoDCMotor.h"
#include "LoHX711.h"

enum {
    KEY_CLICKED = 1,
    KEY_LONG_PRESS,
    KEY_LONG_RELEASE,
};

typedef struct _AppMsg_t {
    MadU8 type;
    union {
        //
    };
    
} AppMsg_t;

#define MM_MSG_TIMEOUT 100

static clocker MM_Clk;
static timer   MM_Timer;
static struct pt MM_Pt;
static MadMsgQCB_t *MM_msgQ;
static LoDCMotor_t MM_Axis0;
static MadFBuffer_t *MM_msgL;

static void app(MadVptr exData);
static void event(MadVptr exData);
static void key_msg(MadU8 type);

static char pt_clean(timer *t);

static void MM_AXIS0_IRQHandler(void) { LoDCMotor_IRQHandler(MM_AXIS0_TIM, &MM_Axis0, MNULL); }

void srvApp_Init(void)
{
    GPIO_InitTypeDef pin;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
    GPIO_PinRemapConfig(GPIO_Remap_TIM4, ENABLE);

    pin.GPIO_Mode  = GPIO_Mode_AF_PP;
    pin.GPIO_Pin   = MM_AXIS0_PWM_P;
    GPIO_Init(MM_AXIS0_PWM_G, &pin);

    LoDCMotor_TimInit(MM_AXIS0_TIM, MM_AXIS0_IRQHandler, MM_AXIS0_IRQn);
    MM_Axis0.t = MM_AXIS0_TIM;
    MM_Axis0.g = MM_AXIS0_DIR_G;
    MM_Axis0.c = MM_AXIS0_CHL;
    MM_Axis0.p = MM_AXIS0_DIR_P;
    LoDCMotor_Init(&MM_Axis0);

    LoHX711_Init();

    MM_msgQ = madMsgQCreate(8);
    MM_msgL = madFBufferCreate(8, sizeof(AppMsg_t));

    madThreadCreate(app,   0, 1024 * 2, THREAD_PRIO_SRV_APP);
    madThreadCreate(event, 0, 1024 * 1, THREAD_PRIO_SRV_EVENT);
}

static void app(MadVptr exData)
{
    MadU8 rc, f_clean, f_dir;
    MadTime_t dt;
    AppMsg_t *msg;
    MadS8 speed;

    (void) exData;
    MAD_LOG("[srvApp]Go!!!\n");

    clocker_init(&MM_Clk);
    tmr_init(&MM_Timer);
    tmr_add(&MM_Timer, &MM_Clk);

    f_clean = MFALSE;
    f_dir = MFALSE;
    speed = 0;

    while(1) {
        rc = madMsgWait(&MM_msgQ, (void**)&msg, MM_MSG_TIMEOUT);

        switch(rc) {
            case MAD_ERR_OK: {
                MAD_CS_OPT(
                    dt = MM_MSG_TIMEOUT - MadCurTCB->timeCntRemain;
                );

                switch(msg->type) {
                    case KEY_CLICKED: {
                        f_clean = !f_clean;
                        if(f_clean) {
                            LoDCMotor_Go(&MM_Axis0, speed);
                            speed = 0;
                        } else {
                            speed = MM_Axis0.speed;
                            LoDCMotor_Go(&MM_Axis0, 0);
                        }
                        break;
                    }

                    case KEY_LONG_PRESS: {
                        f_clean = MFALSE;
                        speed = 0;
                        PT_INIT(&MM_Pt);
                        f_dir = !f_dir;
                        if(f_dir) {
                            LoDCMotor_Go(&MM_Axis0, 100);
                        } else {
                            LoDCMotor_Go(&MM_Axis0, -100);
                        }
                        break;
                    }

                    case KEY_LONG_RELEASE: {
                        LoDCMotor_Go(&MM_Axis0, 0);
                    }

                    default: break;
                }

                madFBufferPut(MM_msgL, msg);
                break;
            }

            case MAD_ERR_TIMEOUT:
                dt = MM_MSG_TIMEOUT;
                break;

            default:
                dt = 0;
                break;
        }

        if(f_clean) {
            clocker_dt(&MM_Clk, dt);
            if(PT_ENDED == pt_clean(&MM_Timer)) {
                f_clean = MFALSE;
                speed = 0;
            }
        }
    }
}

static char pt_clean(timer *t)
{
    PT_BEGIN(&MM_Pt);

    LoDCMotor_Go(&MM_Axis0, 100);
    tmr_set(t, 29 * 1000);
    PT_WAIT_UNTIL(&MM_Pt, tmr_expired(t));

    LoDCMotor_Go(&MM_Axis0, -100);
    tmr_set(t, 29 * 1000);
    PT_WAIT_UNTIL(&MM_Pt, tmr_expired(t));

    LoDCMotor_Go(&MM_Axis0, 0);

    PT_END(&MM_Pt);
}

static void key_msg(MadU8 type)
{
    AppMsg_t *msg = madFBufferGet(MM_msgL);
    if(msg) {
        msg->type = type;
        if(MAD_ERR_OK != madMsgSend(&MM_msgQ, msg)) {
            madFBufferPut(MM_msgL, msg);
        }
    }
}

static void event(MadVptr exData)
{
    int t_key, weight, t_weight;
    GPIO_InitTypeDef pin;

    pin.GPIO_Mode  = GPIO_Mode_IPU;
	pin.GPIO_Speed = GPIO_Speed_50MHz;
    pin.GPIO_Pin   = GPIO_Pin_13;
	GPIO_Init(GPIOD, &pin);

    t_key    = 0;
    weight   = 0;
    t_weight = 0;

    while(1) {
        madTimeDly(10);

        if(Bit_RESET == GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_13)) {
            if(t_key <= 200) {
                if(t_key == 200) {
                    key_msg(KEY_LONG_PRESS);
                }
                t_key++;
            }
        } else {
            if(t_key > 200) {
                key_msg(KEY_LONG_RELEASE);
            } else if(t_key > 5) {
                key_msg(KEY_CLICKED);
            }
            t_key = 0;
        }

        if(t_weight++ > 100 && PT_ENDED == pt_LoHx711_Read(&weight)) {
            // int ofs = 0x3C000 - weight;
            MAD_LOG("Cat's Weight = %d[%d]\n", weight, t_weight);
            t_weight = 0;
        }
    }
}
