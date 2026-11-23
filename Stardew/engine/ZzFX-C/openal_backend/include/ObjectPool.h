#ifndef  OBJECT_POOL_H
#define OBJECT_POOL_H
#ifdef __cplusplus
extern "C"{
#endif 

#include "IntTypes.h"

void* InitObjectPool(int objectSize, int poolInitialSize);

void* GetObjectPoolIndex(void* pObjectPool, int* pOutIndex);

void FreeObjectPoolIndex(void* pObjectPool, int indexToFree);

void* FreeObjectPool(void* pObjectPool);

/// @brief 16 byte aligned
struct ObjectPoolData
{
	i64 objectSize;
	i64 capacity;
	i64 freeObjectsArraySize;
	u64* freeObjectIndicessArray;
};

#define OBJECT_POOL(a) a*
#define NEW_OBJECT_POOL(a, size) ((a*)InitObjectPool(sizeof(a),size))

#define ObjectPoolCapacity(pObjectPool) ((((struct ObjectPoolData*)pObjectPool) - 1)->capacity)

#define OBJ_POOL_BOUNDS_CHECK(handle, rVal, pool)\
{\
	bool bBoundsValid = handle < ObjectPoolCapacity(pool) && handle >= 0;\
	if(!bBoundsValid){\
		printf("function '%s' invalid bounds handle %i", __FUNCTION__, handle);\
		return rVal;\
	}\
}

#define OBJ_POOL_BOUNDS_CHECK_NO_RETURN(handle, pool)\
{\
	bool bBoundsValid = handle < ObjectPoolCapacity(pool) && handle >= 0;\
	if(!bBoundsValid){\
		printf("function '%s' invalid bounds handle %i", __FUNCTION__, handle);\
		return;\
	}\
}

#define ObjectPoolFreeArraySize(pObjectPool) (((struct ObjectPoolData*)pObjectPool) - 1)->freeObjectsArraySize

#ifdef __cplusplus
}
#endif
#endif // ! OBJECT_POOL_H
