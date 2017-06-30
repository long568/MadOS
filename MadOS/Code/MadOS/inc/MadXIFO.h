#ifndef __MAD_XIFO__H__
#define __MAD_XIFO__H__

#include "MadGlobal.h"

/* Mad LIFO ---------------------------------------------*/
#define MAD_LIFO_DECLARE(name, type, size)       \
      type    name##_MAD_LIFO_DATA[size] = {0};  \
      MadInt  name##_MAD_LIFO_INDEX      = -1;   \
const MadInt  name##_MAD_LIFO_MAX        = size

#define MAD_LIFO_EXTERN(name, type)           \
extern       type    name##_MAD_LIFO_DATA[];  \
extern       MadInt  name##_MAD_LIFO_INDEX;   \
extern const MadInt  name##_MAD_LIFO_MAX

#define MAD_LIFO_IS_EMPTY(name)    (name##_MAD_LIFO_INDEX == -1)
#define MAD_LIFO_IS_FULL(name)     (name##_MAD_LIFO_INDEX == (name##_MAD_LIFO_MAX - 1))
#define MAD_LIFO_CNT(name)         (name##_MAD_LIFO_INDEX + 1)
#define MAD_LIFO_MAX(name)         (name##_MAD_LIFO_MAX)
#define MAD_LIFO_DATA(name)        (name##_MAD_LIFO_DATA)
#define MAD_LIFO_CLEAN(name)       do{name##_MAD_LIFO_INDEX = -1;}while(0)
#define MAD_LIFO_PUT(name, value)  do{name##_MAD_LIFO_DATA[++name##_MAD_LIFO_INDEX] = value;}while(0)
#define MAD_LIFO_GET(name, value)  do{value = name##_MAD_LIFO_DATA[name##_MAD_LIFO_INDEX--];}while(0)
/*--------------------------------------------- Mad LIFO */

/* Mad FIFO ---------------------------------------------*/
#define MAD_FIFO_DECLARE(name, type, size)      \
      type   name##_MAD_FIFO_DATA[size] = {0};  \
      MadInt name##_MAD_FIFO_TOP        = 0;    \
      MadInt name##_MAD_FIFO_TAIL       = 0;    \
      MadInt name##_MAD_FIFO_CNT        = 0;    \
const MadInt name##_MAD_FIFO_MAX        = size

#define MAD_FIFO_EXTERN(name, type)          \
extern       type   name##_MAD_FIFO_DATA[];  \
extern       MadInt name##_MAD_FIFO_TOP;     \
extern       MadInt name##_MAD_FIFO_TAIL;    \
extern       MadInt name##_MAD_FIFO_CNT;     \
extern const MadInt name##_MAD_FIFO_MAX
   
#define MAD_FIFO_IS_EMPTY(name)   (name##_MAD_FIFO_CNT == 0)
#define MAD_FIFO_IS_FULL(name)    (name##_MAD_FIFO_CNT == name##_MAD_FIFO_MAX)
#define MAD_FIFO_CNT(name)        (name##_MAD_FIFO_CNT)
#define MAD_FIFO_MAX(name)        (name##_MAD_FIFO_MAX)
#define MAD_FIFO_DATA(name)       (name##_MAD_FIFO_DATA)

#define MAD_FIFO_CLEAN(name)   \
do{                            \
    name##_MAD_FIFO_TOP  = 0;  \
    name##_MAD_FIFO_TAIL = 0;  \
    name##_MAD_FIFO_CNT  = 0;  \
}while(0)  /* MAD_FIFO_CLEAN */

#define MAD_FIFO_PUT(name, value)                        \
do{                                                      \
    name##_MAD_FIFO_DATA[name##_MAD_FIFO_TAIL] = value;  \
    name##_MAD_FIFO_CNT++;                               \
    if(++name##_MAD_FIFO_TAIL == name##_MAD_FIFO_MAX)    \
        name##_MAD_FIFO_TAIL = 0;                        \
}while(0)  /* MAD_FIFO_PUT */

#define MAD_FIFO_GET(name, value)                       \
do{                                                     \
    value = name##_MAD_FIFO_DATA[name##_MAD_FIFO_TOP];  \
    name##_MAD_FIFO_CNT--;                              \
    if(++name##_MAD_FIFO_TOP == name##_MAD_FIFO_MAX)    \
        name##_MAD_FIFO_TOP = 0;                        \
}while(0)  /* MAD_FIFO_GET */

#define MAD_FIFO_UNGET(name, value)                     \
do{                                                     \
    if(--name##_MAD_FIFO_TOP == -1)                     \
        name##_MAD_FIFO_TOP = name##_MAD_FIFO_MAX - 1;  \
    name##_MAD_FIFO_DATA[name##_MAD_FIFO_TOP] = value;  \
    name##_MAD_FIFO_CNT++;                              \
}while(0)  /* MAD_FIFO_UNGET */

#define MAD_FIFO_ARRANGE(name, type)                                                      \
do{                                                                                       \
    if(name##_MAD_FIFO_TOP != 0) {                                                        \
        MadInt i;                                                                         \
        if(name##_MAD_FIFO_TOP < name##_MAD_FIFO_TAIL) {                                  \
            for(i = 0; i < name##_MAD_FIFO_CNT; i++)                                      \
                name##_MAD_FIFO_DATA[i] = name##_MAD_FIFO_DATA[name##_MAD_FIFO_TOP + i];  \
        } else {                                                                          \
            MadInt rn = name##_MAD_FIFO_MAX - name##_MAD_FIFO_TOP;                        \
            type name##_MAD_FIFO_TMP[name##_MAD_FIFO_MAX];                                \
            for(i = 0; i < rn; i++)                                                       \
                name##_MAD_FIFO_TMP[i] = name##_MAD_FIFO_DATA[name##_MAD_FIFO_TOP + i];   \
            for(i = 0; i < name##_MAD_FIFO_TAIL; i++)                                     \
                name##_MAD_FIFO_DATA[rn + i] = name##_MAD_FIFO_DATA[i];                   \
            for(i = 0; i < rn; i++)                                                       \
                name##_MAD_FIFO_DATA[i] = name##_MAD_FIFO_TMP[i];                         \
        }                                                                                 \
    }                                                                                     \
    name##_MAD_FIFO_TOP  = 0;                                                             \
    name##_MAD_FIFO_TAIL = name##_MAD_FIFO_CNT;                                           \
}while(0)  /* MAD_FIFO_ARRANGE */
/*--------------------------------------------- Mad FIFO */

#endif
