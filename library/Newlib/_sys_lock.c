#define __lock _MadMutexCB_t
#include <sys/lock.h>
#include <sys/reent.h>
#include "MadOS.h"

_LOCK_T __lock___sinit_recursive_mutex;
_LOCK_T __lock___sfp_recursive_mutex;
_LOCK_T __lock___atexit_recursive_mutex;
_LOCK_T __lock___at_quick_exit_mutex;
_LOCK_T __lock___malloc_recursive_mutex;
_LOCK_T __lock___env_recursive_mutex;
_LOCK_T __lock___tz_mutex;
_LOCK_T __lock___dd_hash_mutex;
_LOCK_T __lock___arc4random_mutex;

#define NL_MUTEX_CHECK() ((MAD_ERR_OK == madMutexCheck(&lock)) ? 1 : -1)

inline void __retarget_lock_init                 (_LOCK_T *lock) { *lock = madMutexCreate(); }
inline void __retarget_lock_init_recursive       (_LOCK_T *lock) { *lock = madMutexCreateRecursive(); }
inline void __retarget_lock_close                (_LOCK_T lock)  { madMutexDelete(&lock); }
inline void __retarget_lock_close_recursive      (_LOCK_T lock)  { madMutexDelete(&lock); }
inline void __retarget_lock_acquire              (_LOCK_T lock)  { madMutexWait(&lock, 0); }
inline void __retarget_lock_acquire_recursive    (_LOCK_T lock)  { madMutexWait(&lock, 0); }
inline int  __retarget_lock_try_acquire          (_LOCK_T lock)  { return NL_MUTEX_CHECK(); }
inline int  __retarget_lock_try_acquire_recursive(_LOCK_T lock)  { return NL_MUTEX_CHECK(); }
inline void __retarget_lock_release              (_LOCK_T lock)  { madMutexRelease(&lock); }
inline void __retarget_lock_release_recursive    (_LOCK_T lock)  { madMutexRelease(&lock); }

inline void __malloc_lock  (struct _reent *r) { (void)r; }
inline void __malloc_unlock(struct _reent *r) { (void)r; }
