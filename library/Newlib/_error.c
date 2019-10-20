#include "MadOS.h"

int *__errno(void) {
    int *rc;
    MAD_CS_OPT(
        rc = &MadCurTCB->posix_errno;
    );
    return rc;
}
