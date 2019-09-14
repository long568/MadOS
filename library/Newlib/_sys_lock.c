#define __lock _MadMutexCB_t
#include <sys/lock.h>
#include "MadOS.h"

#define NL_MUTEX_CHEXK() ((MTRUE == madMutexCheck(&lock)) ? 1 : -1)

inline void __retarget_lock_init                 (_LOCK_T *lock) { *lock = madMutexCreate(); }
inline void __retarget_lock_init_recursive       (_LOCK_T *lock) { *lock = madMutexCreateRecursive(); }
inline void __retarget_lock_close                (_LOCK_T lock)  { madMutexDelete(&lock); }
inline void __retarget_lock_close_recursive      (_LOCK_T lock)  { madMutexDelete(&lock); }
inline void __retarget_lock_acquire              (_LOCK_T lock)  { madMutexWait(&lock, 0); }
inline void __retarget_lock_acquire_recursive    (_LOCK_T lock)  { madMutexWait(&lock, 0); }
inline int  __retarget_lock_try_acquire          (_LOCK_T lock)  { return NL_MUTEX_CHEXK(); }
inline int  __retarget_lock_try_acquire_recursive(_LOCK_T lock)  { return NL_MUTEX_CHEXK(); }
inline void __retarget_lock_release              (_LOCK_T lock)  { madMutexRelease(&lock); }
inline void __retarget_lock_release_recursive    (_LOCK_T lock)  { madMutexRelease(&lock); }
