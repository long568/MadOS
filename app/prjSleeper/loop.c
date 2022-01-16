#include <stdlib.h>
#include "CfgUser.h"
#include "loop.h"
#include "stabilivolt.h"

static MadMsgQCB_t *msgq;

static void loop_handler(MadVptr exData);

MadBool loop_init(void)
{
    msgq = madMsgQCreate(8);
    if(!msgq) {
        return MFALSE;
    }
    madThreadCreate(loop_handler, 0, 256, THREAD_PRIO_LOOP);
    return MTRUE;
}

MadU8 loop_msg_send(MadVptr msg)
{
    MadU8 rc = madMsgSend(&msgq, msg);
    if(MAD_ERR_OK != rc) {
        free(msg);
    }
    return rc;
}

static void loop_handler(MadVptr exData)
{
    msg_t *msg;
    while (1) {
        madMsgWait(&msgq, (void**)(&msg), 0);
        
        switch (msg->type) {
            case MSG_KEY: {
                switch (msg->arg.v) {
                    case MSG_KEY_SHORT: {
                        sv_add();
                        break;
                    }

                    case MSG_KEY_LONG: {
                        sv_clr();
                        hw_shutdown();
                        break;
                    }

                    default: break;
                }
                break;
            }
            
            case MSG_BLE_SLEEP: {
                sv_set(msg->arg.v);
                break;
            }

            case MSG_BLE_HR: {
                break;
            }

            case MSG_BLE_EQ: {
                break;
            }
            
            default: break;
        }

        free(msg);
    }
}
