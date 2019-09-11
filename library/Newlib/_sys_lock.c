#define __lock _MadMutexCB_t
#include <sys/lock.h>
#include "MadOS.h"

void __retarget_lock_init                 (_LOCK_T *lock) { *lock = madMutexCreate(); }
void __retarget_lock_init_recursive       (_LOCK_T *lock) { *lock = madMutexCreateRecursive(); }
void __retarget_lock_close                (_LOCK_T lock)  { madMutexDelete(&lock); }
void __retarget_lock_close_recursive      (_LOCK_T lock)  { madMutexDelete(&lock); }
void __retarget_lock_acquire              (_LOCK_T lock)  { madMutexWait(&lock, 0); }
void __retarget_lock_acquire_recursive    (_LOCK_T lock)  { madMutexWait(&lock, 0); }
int  __retarget_lock_try_acquire          (_LOCK_T lock)  { return MTRUE == madMutexCheck(&lock) ? 1 : -1; }
int  __retarget_lock_try_acquire_recursive(_LOCK_T lock)  { return MTRUE == madMutexCheck(&lock) ? 1 : -1; }
void __retarget_lock_release              (_LOCK_T lock)  { madMutexRelease(&lock); }
void __retarget_lock_release_recursive    (_LOCK_T lock)  { madMutexRelease(&lock); }
