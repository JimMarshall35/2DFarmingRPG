#include "Thread.h"
#include "AssertLib.h"

#ifdef WIN32

//use win32 threads
CrossPlatformThread StartThread(ThreadFn thread, void* pUser)
{
    DWORD ident = 0;
    HANDLE t = CreateThread( 
            NULL,                   // default security attributes
            0,                      // use default stack size  
            thread,                 // thread function name
            pUser,                  // argument to thread function 
            0,                      // use default creation flags 
            &ident);                // returns the thread identifier 
    return t;
}

void JoinThread(CrossPlatformThread pThread)
{
    DWORD d;
    WaitForSingleObject(pThread, INFINITE);
}

void InitMutex(CrossPlatformMutex* pMtx)
{
    EVERIFY(InitializeCriticalSectionAndSpinCount(pMtx, 0x00000400));
}

void DestroyMutex(CrossPlatformMutex* pMtx)
{
    DeleteCriticalSection(pMtx);
}

void LockMutex(CrossPlatformMutex* pMtx)
{
    EnterCriticalSection(pMtx);
}

void UnlockMutex(CrossPlatformMutex* pMtx)
{
    LeaveCriticalSection(pMtx);
}

CrossPlatformThreadID GetThisThreadsID()
{
    HANDLE hThis = GetCurrentThread();
    return GetThreadId(hThis);
}

void SleepForMS(int ms)
{
    Sleep(ms);
}

#else


//use pthreads



CrossPlatformThread StartThread(ThreadFn threadFn, void* pUser)
{
    pthread_t threadt;
    pthread_create(&threadt, NULL, threadFn, pUser);
    return threadt;
}

void JoinThread(CrossPlatformThread pThread)
{
    void* returnVal;
    pthread_join(pThread, &returnVal);
}

void InitMutex(CrossPlatformMutex* pMtx)
{
    pthread_mutexattr_t attrib;
    pthread_mutexattr_init(&attrib);
    pthread_mutexattr_setpshared(&attrib, PTHREAD_PROCESS_PRIVATE);
    pthread_mutex_init(pMtx, &attrib);
}

void DestroyMutex(CrossPlatformMutex* pMtx)
{
    pthread_mutex_destroy(pMtx);
}

void LockMutex(CrossPlatformMutex* pMtx)
{
    pthread_mutex_lock(pMtx);
}

void UnlockMutex(CrossPlatformMutex* pMtx)
{
    pthread_mutex_unlock(pMtx);
}

#include <sys/syscall.h>

CrossPlatformThreadID GetThisThreadsID()
{
    CrossPlatformThreadID id = syscall(SYS_gettid);
    return id; 
}

void SleepForMS(int ms)
{
    usleep(ms * 1000);
}

#endif