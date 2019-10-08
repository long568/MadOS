#include <netdb.h>

const char ERRS_EAI_NONAME[]     = "EAI_NONAME";
const char ERRS_EAI_SERVICE[]    = "EAI_SERVICE";
const char ERRS_EAI_FAIL[]       = "EAI_FAIL";
const char ERRS_EAI_MEMORY[]     = "EAI_MEMORY";
const char ERRS_EAI_FAMILY[]     = "EAI_FAMILY";
const char ERRS_HOST_NOT_FOUND[] = "HOST_NOT_FOUND";
const char ERRS_NO_DATA[]        = "NO_DATA";
const char ERRS_NO_RECOVERY[]    = "NO_RECOVERY";
const char ERRS_TRY_AGAIN[]      = "TRY_AGAIN";
const char ERRS_UNKNOWN[]        = "Unknown error";

char *gai_strerror(int error)
{
    const char *rc;
    switch (error) {
        case EAI_NONAME:     rc = ERRS_EAI_NONAME;     break;
        case EAI_SERVICE:    rc = ERRS_EAI_SERVICE;    break;
        case EAI_FAIL:       rc = ERRS_EAI_FAIL;       break;
        case EAI_MEMORY:     rc = ERRS_EAI_MEMORY;     break;
        case EAI_FAMILY:     rc = ERRS_EAI_FAMILY;     break;
        case HOST_NOT_FOUND: rc = ERRS_HOST_NOT_FOUND; break;
        case NO_DATA:        rc = ERRS_NO_DATA;        break;
        case NO_RECOVERY:    rc = ERRS_NO_RECOVERY;    break;
        case TRY_AGAIN:      rc = ERRS_TRY_AGAIN;      break;
        default:             rc = ERRS_UNKNOWN;        break;
    }
    return (char*)rc;
}
