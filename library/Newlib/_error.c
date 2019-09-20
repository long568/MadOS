#include "MadOS.h"

inline int *__errno(void) {
    return &MadCurTCB->posix_errno;
}
