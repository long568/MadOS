#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "MadOS.h"
#include "modbus.h"
#include "CfgUser.h"

#define MODBUS_REG_LEN 10

enum {
    MODBUS_RTU = 0,
    MODBUS_TCP
};

const char MODBUS_RTU_HEAD[] = "Modbus-RTU";
const char MODBUS_TCP_HEAD[] = "Modbus-TCP";

static void modbus_client(MadVptr exData);
static int  modbus_read (int fun, modbus_t *ctx, int addr, int len, uint16_t *buf, const char *head);
static int  modbus_write(int fun, modbus_t *ctx, int addr, int val, uint16_t *buf, const char *head);

void Init_TestModbus(void)
{
    madThreadCreate(modbus_client, (MadVptr)MODBUS_RTU, 1024 * 2, THREAD_PRIO_TEST_MODBUS_RTU);
    madThreadCreate(modbus_client, (MadVptr)MODBUS_TCP, 1024 * 2, THREAD_PRIO_TEST_MODBUS_TCP);
}

static void modbus_client(MadVptr exData)
{
    int i, v, rc, type;
    MadU16   *reg_buf;
    modbus_t *ctx;
    const char *head;

    madTimeDly(5000);

    ctx  = 0;
    head = 0;
    type = (int)exData;
    while(1) {
        do {
            rc = 0;
            switch(type) {
                case MODBUS_RTU: ctx = modbus_new_rtu("/dev/tty1", 9600, 'N', 8, 1); head = MODBUS_RTU_HEAD; break;
                case MODBUS_TCP: ctx = modbus_new_tcp("192.168.1.103", 502);         head = MODBUS_TCP_HEAD; break;
                default: madThreadPend(MAD_THREAD_SELF); break;
            }
            modbus_set_debug(ctx, OFF);
            modbus_set_slave(ctx, 1);
            if (modbus_connect(ctx) == -1) {
                MAD_LOG("[%s]Connection failed: %s\n", head, modbus_strerror(errno));
                modbus_free(ctx);
                madTimeDly(3000);
                rc = 1;
            }
        } while(rc);
        reg_buf = malloc(sizeof(MadU16) * MODBUS_REG_LEN);
        MAD_LOG("[%s]Ready...\n", head);

        i = 0;
        v = 0;
        while(1) {
            madTimeDly(1000);
            MAD_LOG("\n[%s]Testing...%d\n", head, v);
            if(0 > modbus_read(3, ctx, 0, MODBUS_REG_LEN, reg_buf, head)) break;
            if(0 > modbus_read(4, ctx, 0, MODBUS_REG_LEN, reg_buf, head)) break;
            if(0 > modbus_write(6, ctx, i, v, 0, head)) break;
            if(++i > 9) i = 0;
            v++;
        }
        modbus_close(ctx);
        modbus_free(ctx);
        free(reg_buf);
        madTimeDly(6000);
    }
}

static int modbus_read(int fun, modbus_t *ctx, int addr, int len, uint16_t *buf, const char *head)
{
    int rc, i;
    
    rc = -1;
    printf("[%s]Read...", head);
    
    switch (fun) {
        case 1: // Coils
            break;
            
        case 2: // Discrete Inputs
            break;
            
        case 3: // Holding Registers
            rc = modbus_read_registers(ctx, addr, len, buf);
            break;
            
        case 4: // Input Registers
            rc = modbus_read_input_registers(ctx, addr, len, buf);
            break;
            
        default:
            break;
    }
    
    if(rc == -1) {
        printf("Error\n");
        memset(buf, 0, len*2);
    } else {
        printf("OK\n");
        for(i=0; i<len; i++) {
            printf("%04X ", buf[i]);
            if((i+1)%5 == 0) printf("\n");
        }
    }
    return rc;
}

static int modbus_write(int fun, modbus_t *ctx, int addr, int val, uint16_t *buf, const char *head)
{
    int rc;
    
    (void)buf;
    rc = -1;
    printf("[%s]Write...", head);
    
    switch (fun) {
        case 5: // Single Coil
            break;
            
        case 6: // Single Register
            rc = modbus_write_register(ctx, addr, val);
            break;
            
        case 15: // Muliple Coils
            break;
            
        case 16: // Muliple Registers
            break;
            
        default:
            break;
    }
    
    if(rc == -1) {
        printf("Error\n");
    } else {
        printf("OK\n");
    }
    return rc;
}
