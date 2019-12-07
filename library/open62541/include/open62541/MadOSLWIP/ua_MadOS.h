#ifndef ARCH_COMMON_MADOS62541_H_
#define ARCH_COMMON_MADOS62541_H_

#include <stdlib.h>
#include <string.h>

#ifdef BYTE_ORDER
# undef BYTE_ORDER
#endif

#define UA_sleep_ms(X) madTimeDly(X)

#define UA_free    free
#define UA_malloc  malloc
#define UA_calloc  calloc
#define UA_realloc realloc

#ifdef UA_ENABLE_DISCOVERY_SEMAPHORE
# ifndef UA_fileExists
#  define UA_fileExists(X) (0)
# endif // UA_fileExists
#endif

// No log colors on MadOS
// #define UA_ENABLE_LOG_COLORS

#include <stdio.h>
#define UA_snprintf snprintf

#define UA_LOG_SOCKET_ERRNO_WRAP(LOG) { \
    char *errno_str = ""; \
    LOG; \
}

#endif /* ARCH_COMMON_MADOS62541_H_ */
