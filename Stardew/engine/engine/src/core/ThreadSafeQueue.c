#include "ThreadSafeQueue.h"
#include <stdlib.h>
#include <string.h>
#include "Log.h"
#include "AssertLib.h"

void TSQ_Init(struct ThreadSafeQueue* pQueue, u32 itemSize, u32 queueSizeItems, OnTSQueueWrapAroundFn wrapAroundCallback)
{
    pQueue->itemSize = itemSize;
    pQueue->queueSize = queueSizeItems;
    pQueue->queueCurrentLength = 0;
    InitMutex(&pQueue->mutex);
    pQueue->data = malloc(itemSize * queueSizeItems);
    memset(pQueue->data, 0, pQueue->itemSize * pQueue->queueSize);
    pQueue->queueHead = pQueue->data;
    pQueue->queueTail = pQueue->data;
    pQueue->onWrap = wrapAroundCallback;
}

void TSQ_DeInit(struct ThreadSafeQueue* pQueue, u32 itemSize, u32 queueSizeItems)
{  
    free(pQueue->data);
}

static void AdvanceQueueTail(struct ThreadSafeQueue* pQueue)
{
    pQueue->queueTail += pQueue->itemSize;
    if(pQueue->queueTail > pQueue->data + (pQueue->queueSize - 1) * pQueue->itemSize)
    {
        pQueue->queueTail = pQueue->data;
    }
}

static void AdvanceQueueHead(struct ThreadSafeQueue* pQueue)
{
    pQueue->queueHead += pQueue->itemSize;
    if(pQueue->queueHead > pQueue->data + (pQueue->queueSize - 1) * pQueue->itemSize)
    {
        pQueue->queueHead = pQueue->data;
    }
}


void TSQ_Enqueue(struct ThreadSafeQueue* pQueue, const void* pIn)
{
    LockMutex(&pQueue->mutex);
    memcpy(pQueue->queueTail, pIn, pQueue->itemSize);
    AdvanceQueueTail(pQueue);
    if(++pQueue->queueCurrentLength > pQueue->queueSize)
    {
        if(pQueue->onWrap)
        {
            pQueue->onWrap(pQueue->queueHead);
        }
        AdvanceQueueHead(pQueue);
        pQueue->queueCurrentLength--;
        Log_Warning("Threadsafe queue is full, discarding first element, consider increasing the queue size, size is: %i", pQueue->queueSize);
    }
    UnlockMutex(&pQueue->mutex);
}

/*
    true if something dequeued, false if queue is empty
*/
bool TSQ_Dequeue(struct ThreadSafeQueue* pQueue, void* pOut)
{
    bool bSomethingDequeued = true;
    LockMutex(&pQueue->mutex);
    if(pQueue->queueCurrentLength == 0)
    {
        bSomethingDequeued = false;
        goto unlock_mutex;
    }
    memcpy(pOut, pQueue->queueHead, pQueue->itemSize);
    AdvanceQueueHead(pQueue);
    pQueue->queueCurrentLength--;
unlock_mutex:
    UnlockMutex(&pQueue->mutex);
    return bSomethingDequeued;
}

