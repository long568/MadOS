#ifndef __BLE_CMD__H__
#define __BLE_CMD__H__

#define  AT  "AT+"
#define  MSG "+"

#define  RESET "Z\r\n"

#define  CHK_NAME  "NAME?\r\n"
#define  SET_NAME  "NAME=%s\r\n"

#define  __AT_CMD(a,x)  write(dev, a x, sizeof(a x) - 1)
#define  AT_CMD(x)      __AT_CMD(AT,x)

#endif
