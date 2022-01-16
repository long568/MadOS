#include <stdlib.h>
#include "CfgUser.h"
#include "key.h"
#include "loop.h"

static MadMutexCB_t *sig;

static void key_irq_handler(void);
static void key_handler(MadVptr exData);

MadBool key_init(void)
{
    LL_EXTI_InitTypeDef EXTI_InitStruct = {0};

    sig = madMutexCreateN();
    if(!sig) {
        return MFALSE;
    }

    EXTI_InitStruct.Line_0_31   = EXTI_KEY_LINE;
    EXTI_InitStruct.LineCommand = ENABLE;
    EXTI_InitStruct.Mode        = LL_EXTI_MODE_IT;
    EXTI_InitStruct.Trigger     = LL_EXTI_TRIGGER_FALLING;
    LL_EXTI_Init(&EXTI_InitStruct);
    madInstallExIrq(key_irq_handler, EXTI_KEY_IRQn, EXTI_KEY_LINE);
    NVIC_SetPriority(EXTI_KEY_IRQn, ISR_PRIO_KEY);
    NVIC_EnableIRQ(EXTI_KEY_IRQn);

    madThreadCreate(key_handler, 0, 256, THREAD_PRIO_KEY);
    return MTRUE;
}

static void key_irq_handler(void)
{
    if (LL_EXTI_IsActiveFallingFlag_0_31(EXTI_KEY_LINE) != RESET) {
        LL_EXTI_ClearFallingFlag_0_31(EXTI_KEY_LINE);
        madMutexRelease(&sig);
    }
}

static void key_handler(MadVptr exData)
{
    int i;
    MadBool err;
    msg_t   *msg;

    while(1) {
        madMutexWait(&sig, 0);

        err = MFALSE;
        for(i=0; i<4; i++) {
            madTimeDly(5);
            if(LL_GPIO_IsInputPinSet(GPIO_KEY, GPIN_KEY)) {
                err = MTRUE;
                break;
            }
        }
        if(err) {
            continue;
        }

        i = 0;
        while(!LL_GPIO_IsInputPinSet(GPIO_KEY, GPIN_KEY)) {
            madTimeDly(5);
            if(++i >= 600) {
                break;
            }
        }

        msg = malloc(sizeof(msg_t));
        if(!msg) {
            continue;
        } else {
            msg->type = MSG_KEY;
        }
        if(i < 600) {
            msg->arg.v = MSG_KEY_SHORT;
        } else {
            msg->arg.v = MSG_KEY_LONG;
        }
        loop_msg_send(msg);
    }
}
