#include "ObjectPool.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "IntTypes.h"

void* InitObjectPool(int objectSize, int poolInitialSize)
{
	struct ObjectPoolData* pAlloc = malloc(objectSize * poolInitialSize + poolInitialSize * sizeof(u64) + sizeof(struct ObjectPoolData));
	if (!pAlloc) { assert(false); return NULL; }
	pAlloc->freeObjectIndicessArray = (u64*)((char*)pAlloc + sizeof(struct ObjectPoolData) + poolInitialSize * objectSize);
	pAlloc->capacity = poolInitialSize;
	pAlloc->freeObjectsArraySize = 0;
	pAlloc->objectSize = objectSize;
	for (int i = 0; i < poolInitialSize; i++)
	{
		pAlloc->freeObjectIndicessArray[pAlloc->freeObjectsArraySize++] = i;
	}
	return pAlloc + 1;
}


void* DoubleObjectPoolSize(void* pObjectPool)
{
	struct ObjectPoolData* pData = ((struct ObjectPoolData*)pObjectPool) - 1;
	assert(pData->freeObjectsArraySize == 0);
	int oldCapacity = pData->capacity;
	pData->capacity *= 2;
	struct ObjectPoolData* pNewData = malloc(pData->objectSize * pData->capacity + sizeof(u64) * pData->capacity + sizeof(struct ObjectPoolData));
	memcpy(pNewData, pData, pData->objectSize * pData->capacity + sizeof(struct ObjectPoolData));
	pNewData->freeObjectIndicessArray = (u64*)((char*)pNewData + sizeof(struct ObjectPoolData) + pData->capacity * pData->objectSize);
	memcpy(pNewData->freeObjectIndicessArray, pData->freeObjectIndicessArray, pData->freeObjectsArraySize * sizeof(u64));

	for(int i=oldCapacity; i < pData->capacity; i++)
	{
		pNewData->freeObjectIndicessArray[pNewData->freeObjectsArraySize++] = i;
	}
	free(pData);
	return pNewData + 1;
}


/// <summary>
/// returns the object pool, possibly resized.
/// returns index of a free space in the pool through pOutIndex
/// </summary>
void* GetObjectPoolIndex(void* pObjectPool, int* pOutIndex)
{
	if (ObjectPoolFreeArraySize(pObjectPool) == 0)
	{
		pObjectPool = DoubleObjectPoolSize(pObjectPool);
	}
	struct ObjectPoolData* pData = ((struct ObjectPoolData*)pObjectPool) - 1;
	*pOutIndex = pData->freeObjectIndicessArray[--pData->freeObjectsArraySize];
	return pObjectPool;
}

void FreeObjectPoolIndex(void* pObjectPool, int indexToFree)
{
	struct ObjectPoolData* pData = ((struct ObjectPoolData*)pObjectPool) - 1;
	if (indexToFree < 0 || indexToFree >= pData->capacity)
	{
		printf("object pool %p index '%i' out of range", pObjectPool, indexToFree);
		return;
	}
	for (int i = 0; i < pData->freeObjectsArraySize; i++)
	{
		if (pData->freeObjectIndicessArray[i] == indexToFree)
		{
			printf("object pool %p index '%i' already free!", pObjectPool, indexToFree);
			return;
		}
	}
	pData->freeObjectIndicessArray[pData->freeObjectsArraySize++] = indexToFree;
}

void* FreeObjectPool(void* pObjectPool)
{
	struct ObjectPoolData* pData = ((struct ObjectPoolData*)pObjectPool) - 1;
	free(pData);
}