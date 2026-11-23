#ifndef TIMERPOOL_H
#define TIMERPOOL_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "HandleDefs.h"

struct SDTimer;


/// @brief return true for the timer to be removed
typedef bool(*TimerCallbackFn)(struct SDTimer*);

/// @brief SD == stardew to avoid name collisions
struct SDTimer
{
    bool bActive;
    bool bRepeat;
    bool bAutoReset;
    double total;

    TimerCallbackFn fnCallback;
    void* pUserData;
    
    // implementation details
    double elapsed;
    HTimer HNext;
    HTimer HPrev;
};


struct SDTimerPool
{
    struct SDTimer* pPool;
    HTimer hTimerListhead;
    HTimer hTimerListend;
};

struct SDTimerPool TP_InitTimerPool(int initialSize);
struct SDTimerPool TP_DestroyTimerPool(struct SDTimerPool* pPool);

HTimer TP_GetTimer(struct SDTimerPool* pPool, const struct SDTimer* pTimer);
void TP_FreeTimer(struct SDTimerPool* pPool, HTimer timer);

void TP_DoTimers(struct SDTimerPool* pPool, double deltaT);

#ifdef __cplusplus
}
#endif
#endif