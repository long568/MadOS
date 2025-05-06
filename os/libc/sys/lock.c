#include <sys/lock.h>
#include "MadOS.h"

struct __lock; typedef struct __lock* _LOCK_T;
extern struct __lock __libc_recursive_mutex;

struct __lock {
    MadMutexCB_t* mm;
};

inline void __retarget_lock_init              (_LOCK_T *lock) { (*lock)->mm = madMutexCreate(); }
inline void __retarget_lock_init_recursive    (_LOCK_T *lock) { (*lock)->mm = madMutexCreateRecursive(); }
inline void __retarget_lock_close             (_LOCK_T lock)  { madMutexDelete(&(lock->mm)); }
inline void __retarget_lock_close_recursive   (_LOCK_T lock)  { madMutexDelete(&(lock->mm)); }
inline void __retarget_lock_acquire           (_LOCK_T lock)  { madMutexWait(&(lock->mm), 0); }
inline void __retarget_lock_acquire_recursive (_LOCK_T lock)  { madMutexWait(&(lock->mm), 0); }
inline void __retarget_lock_release           (_LOCK_T lock)  { madMutexRelease(&(lock->mm)); }
inline void __retarget_lock_release_recursive (_LOCK_T lock)  { madMutexRelease(&(lock->mm)); }
