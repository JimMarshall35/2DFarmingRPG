#ifndef THREADSAFE_QUEUE_H
#define THREADSAFE_QUEUE_H

#include "IntTypes.h"
#include "Thread.h"
#include <stdbool.h>


/// @brief callback for when an item is about to be lost by the queue reaching its maximum size
/// and wrapping around, losing the first item in the queue. An opportunity to properly fre
/// the item in the queue.
typedef void(*OnTSQueueWrapAroundFn)(void* pItemToBeLost);

/// @brief Fixed size queue that wraps around when full
struct ThreadSafeQueue
{
    u8* data;
    u32 itemSize;
    u32 queueSize;
    u32 queueCurrentLength;
    u8* queueHead;
    u8* queueTail;
    CrossPlatformMutex mutex;
    OnTSQueueWrapAroundFn onWrap;
};

/// @brief 
/// @param pQueue 
/// @param itemSize 
/// @param queueSizeItems 
/// @param wrapAroundCallback 
void TSQ_Init(struct ThreadSafeQueue* pQueue, u32 itemSize, u32 queueSizeItems, OnTSQueueWrapAroundFn wrapAroundCallback);

/// @brief 
/// @param pQueue 
/// @param pIn 
void TSQ_Enqueue(struct ThreadSafeQueue* pQueue, const void* pIn);

/// @brief 
/// @param pQueue 
/// @param pOut 
/// @return true if something dequeued, false if queue is empty
bool TSQ_Dequeue(struct ThreadSafeQueue* pQueue, void* pOut);

#endif