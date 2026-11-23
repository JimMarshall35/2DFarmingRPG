#include <gtest/gtest.h>
#include "ObjectPool.h"

template<typename T>
struct ScopedObjectPool
{
    ScopedObjectPool(int size)
        :pPool(NEW_OBJECT_POOL(T, size))
    {

    }
    ~ScopedObjectPool()
    {
        FreeObjectPool(pPool);
    }
    OBJECT_POOL(T) pPool;
};

TEST(ObjectPool, GetIndexNoRealloc)
{
    ScopedObjectPool<int> pool{5};
    struct 
    {
        int value;
        int hIndex;
    }poolAllocations[5];
    
    int testValues[5] = {
        42,
        53,
        6342,
        4,
        754
    };
    int onAllocation = 0;
    for(int i=0; i<5; i++)
    {
        pool.pPool = (int*)GetObjectPoolIndex(pool.pPool,&poolAllocations[i].hIndex);
        pool.pPool[poolAllocations[i].hIndex] = testValues[i];
    }

    for(int i=0; i<5; i++)
    {
        ASSERT_EQ(pool.pPool[poolAllocations[i].hIndex], testValues[i]);
    }
}

TEST(ObjectPool, GetIndexRealloc)
{
    ScopedObjectPool<int> pool{5};
    struct 
    {
        int value;
        int hIndex;
    }poolAllocations[12];
    
    int testValues[12] = {
        42,
        53,
        6342,
        4,
        754,

        12,
        42,
        55,
        57,
        99,
        1,
        53235, 
    };

    ASSERT_EQ(ObjectPoolCapacity(pool.pPool), 5);

    int onAllocation = 0;
    for(int i=0; i<12; i++)
    {
        pool.pPool = (int*)GetObjectPoolIndex(pool.pPool,&poolAllocations[i].hIndex);
        pool.pPool[poolAllocations[i].hIndex] = testValues[i];
    }
    ASSERT_EQ(ObjectPoolCapacity(pool.pPool), 20);
    ASSERT_EQ(ObjectPoolFreeArraySize(pool.pPool), 8);

    for(int i=0; i<12; i++)
    {
        ASSERT_EQ(pool.pPool[poolAllocations[i].hIndex], testValues[i]);
    }
}

TEST(ObjectPool, FreeObjectPoolIndex)
{
    ScopedObjectPool<int> pool{5};
    struct 
    {
        int value;
        int hIndex;
    }poolAllocations[5];
    
    int testValues[5] = {
        42,
        53,
        6342,
        4,
        754,
    };

    for(int i=0; i<5; i++)
    {
        pool.pPool = (int*)GetObjectPoolIndex(pool.pPool,&poolAllocations[i].hIndex);
        pool.pPool[poolAllocations[i].hIndex] = testValues[i];
    }

    ASSERT_EQ(ObjectPoolCapacity(pool.pPool), 5);

    FreeObjectPoolIndex(pool.pPool, poolAllocations[2].hIndex);
    FreeObjectPoolIndex(pool.pPool, poolAllocations[3].hIndex);
    
    GetObjectPoolIndex(pool.pPool, &poolAllocations[2].hIndex);
    GetObjectPoolIndex(pool.pPool, &poolAllocations[3].hIndex);

    ASSERT_EQ(ObjectPoolCapacity(pool.pPool), 5);
    
}