#include "MadOS.h"

int *__errno(void) {
    int *rc;
    MAD_PROTECT_OPT(
        rc = &MadCurTCB->posix_errno;
    );
    return rc;
}
