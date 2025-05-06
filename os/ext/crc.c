#include "crc.h"

// x^7 + x^3 + 1
#define CRC7_h 0x09
MadU8 CRC7(const MadU8 *data, int len)
{
    MadU8 i, j;
    MadU8 crc = 0x00;
    for ( i = 0; i < len; i++){
        crc ^= data[i];
        for (j = 0; j < 8; j++){
            if (crc &0x80){
                crc ^= CRC7_h;
            }
            crc = crc <<1;
        }
    }
    crc = crc >> 1;
    return crc;
}

// x^16 + x^15 + x^2 + 1
#define CRC16_h 0xA001
MadU16 CRC16(const MadU8 *data, int len)
{
    int i, j, k;
    MadU16 crc = 0xFFFF;
    for(i=0; i<len; i++) {
        crc = crc ^ (MadU16)data[i];
        for(j=0; j<8; j++) {
            k = crc & 0x01;
            crc >>= 1;
            if(k) {
                crc ^= CRC16_h;
            }
        }
    }
    return crc;
}
