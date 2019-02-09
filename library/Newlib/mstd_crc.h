#ifndef __MSTD_CRC__H__
#define __MSTD_CRC__H__

#include "MadOS.h"

MadU8  CRC7 (const MadU8 *data, int len);
MadU16 CRC16(const MadU8 *data, int len);

#endif
