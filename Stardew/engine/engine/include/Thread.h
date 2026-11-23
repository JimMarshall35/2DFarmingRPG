#ifndef STARDEW_THREAD_H
#define STARDEW_THREAD_H

#ifdef WIN32

#include <windows.h>
typedef HANDLE CrossPlatformThread;

typedef DWORD(*ThreadFn)(void*);

#define DECLARE_THREAD_PROC(name, argName) DWORD name (void* argName)

typedef CRITICAL_SECTION CrossPlatformMutex;

typedef DWORD CrossPlatformThreadID;

#else

#include <pthread.h>
#define _GNU_SOURCE
#include <unistd.h>
#include <sys/types.h>

/// @brief Thread functions have return values as required by the OS thread libs but
/// as the win32 thead returns a dword (32bit) and the pthread returns a void* (64 bit),
/// so our abstraction just ignores return values
typedef void*(*ThreadFn)(void*);

typedef pthread_t CrossPlatformThread;

#define DECLARE_THREAD_PROC(name, argName) void* name (void* argName)

/// @brief Mutexes - These will work between threads in the same process: in windows terms, a "critical section"
typedef pthread_mutex_t CrossPlatformMutex;

typedef pid_t CrossPlatformThreadID;

#endif


/// @brief Thread functions have return values as required by the OS thread libs but
/// as the win32 thead returns a dword (32bit) and the pthread returns a void* (64 bit),
/// so our abstraction just ignores return values
/// @param thread 
/// @param pUser 
/// @return 
CrossPlatformThread StartThread(ThreadFn thread, void* pUser);

void JoinThread(CrossPlatformThread pThread);

/*
    Mutexes
    - These will work between threads in the same process: in windows terms, a "critical section"
*/

/// @brief 
/// @param pMtx 
void InitMutex(CrossPlatformMutex* pMtx);

/// @brief 
/// @param pMtx 
void DestroyMutex(CrossPlatformMutex* pMtx);

/// @brief 
/// @param pMtx 
void LockMutex(CrossPlatformMutex* pMtx);

/// @brief 
/// @param pMtx 
void UnlockMutex(CrossPlatformMutex* pMtx);

/// @brief 
/// @return 
CrossPlatformThreadID GetThisThreadsID();

/// @brief Make the calling thread sleep for ms milliseconds
/// @param ms 
void SleepForMS(int ms);

#endif
