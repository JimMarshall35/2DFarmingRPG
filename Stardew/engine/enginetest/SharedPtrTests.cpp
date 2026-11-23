#include <gtest/gtest.h>
#include "SharedPtr.h"
#include "IntTypes.h"

static int dtorCalls = 0;

static u8* pSPDtorData = NULL; 


void spDtor(void* data)
{
    pSPDtorData = (u8*)data;
    dtorCalls++;
}

TEST(SharedPtr, BasicTest)
{
    volatile u8 val = 0;;
    u8* pData = (u8*)Sptr_New(128, &spDtor); // ref 1
    pData[64] = 'j';
    pData[65] = 'i';
    pData[66] = 'm';
    val = *pData;
    val = pData[12];

    Sptr_AddRef(pData); // ref 2
    pData[64] = 'm';
    pData[65] = 'a';
    pData[66] = 'r';
    val = *pData;
    val = pData[12];

    Sptr_AddRef(pData); // ref 3
    pData[64] = 's';
    pData[65] = 'h';
    pData[66] = 'a';
    val = *pData;
    val = pData[12];

    Sptr_RemoveRef(pData); // ref 2
    pData[64] = 'j';
    pData[65] = 'i';
    pData[66] = 'm';
    val = pData[41];
    val = pData[31];


    Sptr_RemoveRef(pData); // ref 1
    pData[4] = 'j';
    pData[12] = 'i';
    pData[66] = 'm';
    val = *pData;
    val = pData[12];

    Sptr_RemoveRef(pData); // ref 0

    ASSERT_EQ(dtorCalls, 1);
    ASSERT_EQ(pSPDtorData, pData);
}