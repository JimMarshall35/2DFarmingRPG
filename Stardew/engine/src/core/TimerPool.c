#include "TimerPool.h"
#include "ObjectPool.h"
#include <stdlib.h>
#include <string.h>
#include "AssertLib.h"

struct SDTimerPool TP_InitTimerPool(int initialSize)
{
    struct SDTimerPool p;
    p.pPool = NEW_OBJECT_POOL(struct SDTimer, initialSize);
    p.hTimerListhead = NULL_HANDLE;
    p.hTimerListend = NULL_HANDLE;
    return p;
}

struct SDTimerPool TP_DestroyTimerPool(struct SDTimerPool* pPool)
{
    FreeObjectPool(pPool->pPool);
}

HTimer TP_GetTimer(struct SDTimerPool* pPool, const struct SDTimer* pTimer)
{
    HTimer handle = NULL_HANDLE;
    pPool->pPool = GetObjectPoolIndex(pPool->pPool, &handle);
    memcpy(&pPool->pPool[handle], pTimer, sizeof(struct SDTimer));
    struct SDTimer* pTimerOut = &pPool->pPool[handle];
    pTimerOut->HNext = NULL_HANDLE;
    pTimerOut->HPrev = NULL_HANDLE;
    
    if(pPool->hTimerListhead == NULL_HWIDGET)
    {
        pPool->hTimerListhead = handle;
        pPool->hTimerListend = handle;
    }
    else
    {
        EASSERT(pPool->hTimerListend != NULL_HANDLE);
        pTimerOut->HPrev = pPool->hTimerListend;
        pPool->pPool[pPool->hTimerListend].HNext = handle;
    }
    return handle;
}

void TP_FreeTimer(struct SDTimerPool* pPool, HTimer timer)
{
    struct SDTimer* pTimerOut = &pPool->pPool[timer];
    if(pTimerOut->HNext != NULL_HANDLE)
    {
        pPool->pPool[pTimerOut->HNext].HPrev = pTimerOut->HPrev;
        //pTimerOut->pNext->pPrev = pTimerOut->pPrev ? pTimerOut->pPrev : NULL;
    }
    else
    {
        pPool->hTimerListend = pTimerOut->HPrev;
    }
    if(pTimerOut->HPrev != NULL_HANDLE)
    {
        pTimerOut->HPrev = pTimerOut->HNext;
    }
    else
    {
        pPool->hTimerListhead = pPool->pPool[timer].HNext;
    }
    FreeObjectPoolIndex(pPool->pPool, timer);
}

void TP_DoTimers(struct SDTimerPool* pPool, double deltaT)
{
    HTimer head = pPool->hTimerListhead;
    while(head != NULL_HANDLE)
    {
        struct SDTimer* pTimerOut = &pPool->pPool[head];
        bool bRemove = false;
        if(pTimerOut->bActive)
        {
            pTimerOut->elapsed += deltaT;
            if(pTimerOut->elapsed >= pTimerOut->total)
            {
                if(pTimerOut->bAutoReset)
                {
                    pTimerOut->elapsed = 0.0;
                }
                bRemove = pTimerOut->fnCallback(pTimerOut);
            }
        }
        if(bRemove)
        {
            TP_FreeTimer(pPool, head);
        }
        head = pTimerOut->HNext;
        
    }
}