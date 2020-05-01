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

#define srvModbusE_RD  0x0001
#define srvModbusE_WR  0x0002
#define srvModbusE_TO  0x0010
#define srvModbusE_All (srvModbusE_RD | srvModbusE_WR | srvModbusE_TO)

extern MadEventCB_t *srvModbus_Event;

extern void srvModbus_Init(void);

#endif
