#ifndef __HEARTRATE__H__
#define __HEARTRATE__H__

#ifdef __cplusplus
extern "C" {
#endif

#include "MadOS.h"

extern MadBool max_init    (void);
extern int     max_hr_get  (void);
extern int     max_spo2_get(void);

#ifdef __cplusplus
}
#endif

#endif
