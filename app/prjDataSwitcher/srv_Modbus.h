#ifndef __SRV_MODBUS__H__
#define __SRV_MODBUS__H__

/***************************************************
-- Read
Coils -------------- 1 - modbus_read_bits
Discrete Inputs ---- 2 - modbus_read_input_bits    
Holding Registers -- 3 - modbus_read_registers    
Input Registers ---- 4 - modbus_read_input_registers
-- Write
Single Coil -------- 5 - modbus_write_bit
Single Register ---- 6 - modbus_write_register
Muliple Coils ----- 15 - modbus_write_bits
Muliple Registers - 16 - modbus_write_registers
***************************************************/

#define srvModbusE_RD  0x01
#define srvModbusE_WR  0x02
#define srvModbusE_TO  0x10
#define srvModbusE_Nor (srvModbusE_RD | srvModbusE_WR)
#define srvModbusE_All (srvModbusE_RD | srvModbusE_WR | srvModbusE_TO)

#define srvModbus_MSGQSIZ 4
#define srvModbus_MSGQTO  (1000 * 30)
#define srvModbus_BUFSIZ  3600
#define srvModbus_AGVOFS  (3 * sizeof(uint16_t))
#define srvModbus_WADDR   0
#define srvModbus_RADDR   (3600 / 2)
#define srvModbus_STEP    100 // 200 Bytes
#define srvModbus_TIMES   18

typedef struct {
    MadU8 e;
    int   s;
    char *b;
} srvModbus_Msg_t;

extern MadMsgQCB_t  *srvModbus_MsgQ;
extern MadFBuffer_t *srvModbus_MsgG;

extern void srvModbus_Init(void);

#endif
