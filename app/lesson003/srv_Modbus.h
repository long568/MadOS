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

extern void srvModbus_Init(void);

#endif
