#include "MadOS.h"
#include "pt.h"
#include "timer.h"
#include "CfgUser.h"
#include "srv_App.h"
#include "LoDCMotor.h"
#include "LoHX711.h"

enum {
    KEY_SHORT = 0,
    KEY_LONG,
};

#define MM_MSG_TIMEOUT 100

static clocker MM_Clk;
static timer   MM_Timer;
static struct pt MM_Pt;
static MadMsgQCB_t *MM_msgQ;
static LoDCMotor_t MM_Axis0;

static void app(MadVptr exData);
static void key(MadVptr exData);

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
    madThreadCreate(app, 0, 1024 * 2, THREAD_PRIO_SRV_APP);
    madThreadCreate(key, 0, 1024 * 1, THREAD_PRIO_SRV_KEY);
}

static void app(MadVptr exData)
{
    MadU8 rc, f_clean;
    MadTim_t dt;
    MadVptr msg;
    MadS8 speed;

    (void) exData;
    MAD_LOG("[srvApp]Go!!!\n");

    clocker_init(&MM_Clk);
    timer_init(&MM_Timer);
    timer_add(&MM_Timer, &MM_Clk);

    f_clean = MFALSE;
    speed = 0;

    while(1) {
        rc = madMsgWait(&MM_msgQ, &msg, MM_MSG_TIMEOUT);

        switch(rc) {
            case MAD_ERR_OK: {
                MAD_CS_OPT(
                    dt = MM_MSG_TIMEOUT - MadCurTCB->timeCntRemain;
                );
                switch((int)msg) {
                    case KEY_SHORT: {
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

                    case KEY_LONG: {
                        f_clean = MFALSE;
                        speed = 0;
                        PT_INIT(&MM_Pt);
                        break;
                    }
                }

                MAD_LOG("[srvApp]Key = %d | Speed = %d\n", (int)msg, MM_Axis0.speed);
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
    timer_set(t, 29 * 1000);
    PT_WAIT_UNTIL(&MM_Pt, timer_expired(t));

    LoDCMotor_Go(&MM_Axis0, -100);
    timer_set(t, 29 * 1000);
    PT_WAIT_UNTIL(&MM_Pt, timer_expired(t));

    LoDCMotor_Go(&MM_Axis0, 0);

    PT_END(&MM_Pt);
}

static void key(MadVptr exData)
{
    int cnt, weight, t_weight;
    MadU8  f_dir;
    GPIO_InitTypeDef pin;

    pin.GPIO_Mode  = GPIO_Mode_IPU;
	pin.GPIO_Speed = GPIO_Speed_50MHz;
    pin.GPIO_Pin   = GPIO_Pin_13;
	GPIO_Init(GPIOD, &pin);

    cnt   = 0;
    f_dir = 0;
    weight = 0;
    t_weight = 0;

    while(1) {
        madTimeDly(10);
        if(Bit_RESET == GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_13)) {
            if(cnt <= 300) {
                if(cnt == 300) {
                    MadS8 speed;
                    f_dir = !f_dir;
                    if(f_dir) speed = 100;
                    else speed = -100;
                    LoDCMotor_Go(&MM_Axis0, speed);
                }
                cnt++;
            }
        } else {
            if(cnt >= 300) {
                LoDCMotor_Go(&MM_Axis0, 0);
                madMsgSend(&MM_msgQ, (MadVptr)KEY_LONG);
            } else if(cnt > 5) {
                madMsgSend(&MM_msgQ, (MadVptr)KEY_SHORT);
            }
            cnt = 0;
        }

        if(t_weight++ > 100 && PT_ENDED == pt_LoHx711_Read(&weight)) {
            int ofs = 0x3C000 - weight;
            MAD_LOG("Cat's Weight = %d[%d]\n", ofs, t_weight);
            t_weight = 0;
        }
    }
}
