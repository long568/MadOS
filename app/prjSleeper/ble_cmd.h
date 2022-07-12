#ifndef __BLE_CMD__H__
#define __BLE_CMD__H__

#define  HELLO       "Hello Pong!\r\n"

#define  AT          "\r\nAT+"
#define  MSG         "\r\n+"
#define  MSG_DATA    "\r\n+DATA"
#define  MSG_CONN    "\r\n+CONN"
#define  MSG_DISCONN "\r\n+DISCONN"
#define  MSG_MAC_ADR "\r\n+MAC:" //"\r\n+MAC:%s\r\nOK\r\n"

#define  SET_TT      "+++\r\n"
#define  EXT_TT      "+++"
#define  SET_AUTO_TT "AUTO+++=%c\r\n"
#define  CHK_MAC_ADR "MAC?\r\n"

#define  CHK_NAME    "NAME?\r\n"
#define  SET_NAME    "NAME=%s\r\n"

#define  AT_CMD(cmd, args...)  \
do {                                    \
    cnt = sprintf(buf, AT cmd, ##args); \
    write(dev, buf, cnt);               \
} while(0)

#define AT_RD_NONE() \
cnt = read(dev, buf, BUF_SIZE); \
if(cnt < 0) {                   \
    close(dev);                 \
    continue;                   \
}

#endif
