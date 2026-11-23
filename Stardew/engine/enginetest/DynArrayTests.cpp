#include <gtest/gtest.h>
#include "DynArray.h"

TEST(Vector, VectorPush)
{
    VECTOR(int) test = NEW_VECTOR(int);
    int val = 13;
    for(int i=0; i<500; i++)
    {
        ASSERT_EQ(i, VectorSize(test));
        test = (int*)VectorPush(test, &val);
    }
    ASSERT_EQ(500, VectorSize(test));
    DestoryVector(test);
}

TEST(Vector, VectorPop)
{
    VECTOR(int) test = NEW_VECTOR(int);
    for(int i=0; i<500; i++)
    {
        test = (int*)VectorPush(test, &i);
    }
    for(int i=499; i>=0; i--)
    {
        ASSERT_EQ(i + 1, VectorSize(test));
        int val = *((int*)VectorPop(test));
        ASSERT_EQ(val, i);
    }
    ASSERT_EQ(0, VectorSize(test));

    DestoryVector(test);
}

TEST(Vector, VectorClear)
{
    VECTOR(int) test = NEW_VECTOR(int);
    for(int i=0; i<187; i++)
    {
        test = (int*)VectorPush(test, &i);
    }
    ASSERT_EQ(187, VectorSize(test));
    VectorData* vd = VectorData_DEBUG(test);
    u32 capacityPreClear = vd->capacity;
    test = (int*)VectorClear(test);
    vd = VectorData_DEBUG(test);

    // should not effect capcity
    ASSERT_EQ(vd->capacity, capacityPreClear);
    // should set size to zero
    ASSERT_EQ(0, VectorSize(test));
    DestoryVector(test);
}

TEST(Vector, StructType)
{
    // c style struct usage to reflect C useage of code under test
    struct TestStruct
    {
        int a;
        int b;
        float c;
        double d;
    }val{
        1,
        2,
        63.0f,
        253.0
    };
    VECTOR(struct TestStruct) test = NEW_VECTOR(struct TestStruct);

    test = (struct TestStruct*)VectorPush(test, &val);
    val.a = 2;
    val.b = 3;
    val.c = 64.0f;
    val.d = 256.0;
    test = (struct TestStruct*)VectorPush(test, &val);
    
    val = *((struct TestStruct*)VectorPop(test));
    ASSERT_EQ(val.a, 2);
    ASSERT_EQ(val.b, 3);
    ASSERT_EQ(val.c, 64.0f);
    ASSERT_EQ(val.d, 256.0);

    val = *((struct TestStruct*)VectorPop(test));
    ASSERT_EQ(val.a, 1);
    ASSERT_EQ(val.b, 2);
    ASSERT_EQ(val.c, 63.0f);
    ASSERT_EQ(val.d, 253.0);

    DestoryVector(test);
}

TEST(Vector, VectorResize)
{
    VECTOR(char) test = NEW_VECTOR(char);
    
    test = (char*)VectorResize(test, 123);
    VectorData* vd = VectorData_DEBUG(test);
    ASSERT_EQ(vd->capacity, 123);
    ASSERT_EQ(vd->size, 0);
    DestoryVector(test);
}

TEST(Vector, VectorInit)
{
    VECTOR(char) test = NEW_VECTOR(char);

    VectorData* vd = VectorData_DEBUG(test);
    ASSERT_EQ(vd->capacity, 1);
    ASSERT_EQ(vd->size, 0);
    ASSERT_EQ(vd->itemSize, sizeof(char));


    VECTOR(short) test2 = NEW_VECTOR(short);

    vd = VectorData_DEBUG(test2);
    ASSERT_EQ(vd->capacity, 1);
    ASSERT_EQ(vd->size, 0);
    ASSERT_EQ(vd->itemSize, sizeof(short));

    VECTOR(double) test3 = NEW_VECTOR(double);

    vd = VectorData_DEBUG(test3);
    ASSERT_EQ(vd->capacity, 1);
    ASSERT_EQ(vd->size, 0);
    ASSERT_EQ(vd->itemSize, sizeof(double));

    DestoryVector(test);
    DestoryVector(test2);
    DestoryVector(test3);
}

TEST(Vector, VectorCapacityDoublesWhenFull)
{
    VECTOR(char) test = NEW_VECTOR(char);
    VectorData* vd = VectorData_DEBUG(test);

    ASSERT_EQ(vd->capacity, 1);
    char val = 'j';
    test = (char*)VectorPush(test, &val);
    vd = VectorData_DEBUG(test);
    ASSERT_EQ(vd->capacity, 1);

    val = 'i';
    test = (char*)VectorPush(test, &val);
    vd = VectorData_DEBUG(test);
    ASSERT_EQ(vd->capacity, 2);

    val = 'm';
    test = (char*)VectorPush(test, &val);
    vd = VectorData_DEBUG(test);
    ASSERT_EQ(vd->capacity, 4);

    val = 'M';
    test = (char*)VectorPush(test, &val);
    vd = VectorData_DEBUG(test);
    ASSERT_EQ(vd->capacity, 4);

    val = 'M';
    test = (char*)VectorPush(test, &val);
    vd = VectorData_DEBUG(test);
    ASSERT_EQ(vd->capacity, 8);

    val = 'a';
    test = (char*)VectorPush(test, &val);
    vd = VectorData_DEBUG(test);
    ASSERT_EQ(vd->capacity, 8);

    val = 'r';
    test = (char*)VectorPush(test, &val);
    vd = VectorData_DEBUG(test);
    ASSERT_EQ(vd->capacity, 8);

    val = 's';
    test = (char*)VectorPush(test, &val);
    vd = VectorData_DEBUG(test);
    ASSERT_EQ(vd->capacity, 8);

    val = 'h';
    test = (char*)VectorPush(test, &val);
    vd = VectorData_DEBUG(test);
    ASSERT_EQ(vd->capacity, 16);

    val = 'a';
    test = (char*)VectorPush(test, &val);
    vd = VectorData_DEBUG(test);
    ASSERT_EQ(vd->capacity, 16);

    val = 'l';
    test = (char*)VectorPush(test, &val);
    vd = VectorData_DEBUG(test);
    ASSERT_EQ(vd->capacity, 16);

    val = 'l';
    test = (char*)VectorPush(test, &val);
    vd = VectorData_DEBUG(test);
    ASSERT_EQ(vd->capacity, 16);

    DestoryVector(test);
}