#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "MadOS.h"
#include "modbus.h"
#include "CfgUser.h"

#define MODBUS_REG_LEN 10

static void modbus_client(MadVptr exData);
static void modbus_read(int fun, modbus_t *ctx, int addr, int len, uint16_t *buf);
static void modbus_write(int fun, modbus_t *ctx, int addr, int val, uint16_t *buf);

void Init_TestModbus(void)
{
    madThreadCreate(modbus_client, 0, 1024 * 2, THREAD_PRIO_TEST_MODBUS);
}

static void modbus_client(MadVptr exData)
{
    int i, v, rc;
    modbus_t *ctx;
    MadU16   *reg_buf;

    madTimeDly(5000);

    do {
        rc = 0;
        // ctx = modbus_new_rtu("/dev/tty1", 9600, 'N', 8, 1);
        ctx = modbus_new_tcp("192.168.1.103", 502);
        modbus_set_debug(ctx, OFF);
        modbus_set_slave(ctx, 1);
        if (modbus_connect(ctx) == -1) {
            MAD_LOG("[Modbus]Connection failed: %s\n", modbus_strerror(errno));
            modbus_free(ctx);
            madTimeDly(2000);
            rc = 1;
        }
    } while(rc);
    reg_buf = malloc(sizeof(MadU16) * MODBUS_REG_LEN);
    MAD_LOG("[Modbus]Ready...\n");

    i = 0;
    v = 0;
    while(1) {
        madTimeDly(2000);
        MAD_LOG("\n[Modbus]Testing...%d\n", v);
        modbus_read(3, ctx, 0, MODBUS_REG_LEN, reg_buf);
        modbus_read(4, ctx, 0, MODBUS_REG_LEN, reg_buf);
        modbus_write(6, ctx, i, v, 0);
        if(++i > 9) i = 0;
        v++;
    }
}

static void modbus_read(int fun, modbus_t *ctx, int addr, int len, uint16_t *buf)
{
    int rc, i;
    
    rc = -1;
    printf("[Modbus]Read...");
    
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
}

static void modbus_write(int fun, modbus_t *ctx, int addr, int val, uint16_t *buf)
{
    int rc;
    
    (void)buf;
    rc = -1;
    printf("[Modbus]Write...");
    
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
}
