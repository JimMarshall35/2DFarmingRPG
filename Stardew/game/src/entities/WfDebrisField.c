#include "WfDebrisField.h"
#include "Entities.h"
#include "Game2DLayer.h"
#include "ObjectPool.h"
#include "BinarySerializer.h"
#include "AssertLib.h"
#include "GameFramework.h"

struct WfDebrisFieldData
{
    float BigRocksPercentage;
    float Density;
    float LogsPercentage;
    float RocksPercentage;
    float widthPx, heightPx;
};

OBJECT_POOL(struct WfDebrisFieldData) gDebrisFieldPool = NULL;

void WfDebrisFieldInit()
{
    gDebrisFieldPool = NEW_OBJECT_POOL(struct WfDebrisFieldData, 16);
}

void WfDebrisFieldEntityOnInit(struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer, DrawContext* pDrawCtx, InputContext* pInputCtx)
{
    struct GameLayer2DData* pLayerData = pLayer->userData;
    /* destroy the entity */
    Et2D_DestroyEntity(pLayer, &pLayerData->entities, pEnt->thisEntity);
}

void WfDebrisFieldEntityOnDestroy(struct Entity2D* pEnt, struct GameFrameworkLayer* pData)
{
    FreeObjectPoolIndex(gDebrisFieldPool, pEnt->user.hData);
}

void WfDeSerializeDebrisFieldEntity(struct BinarySerializer* bs, struct Entity2D* pOutEnt, struct GameLayer2DData* pData)
{
    i32 version = -1;
    BS_DeSerializeI32(&version, bs);
    switch (version)
    {
    case 1:
        {
            HGeneric hDebrisData = NULL_HANDLE;
            gDebrisFieldPool = GetObjectPoolIndex(gDebrisFieldPool, &hDebrisData);
            struct WfDebrisFieldData* pData = &gDebrisFieldPool[hDebrisData];
            BS_DeSerializeFloat(&pData->BigRocksPercentage, bs);
            BS_DeSerializeFloat(&pData->Density, bs);
            BS_DeSerializeFloat(&pData->LogsPercentage, bs);
            BS_DeSerializeFloat(&pData->RocksPercentage, bs);
            BS_DeSerializeFloat(&pData->widthPx, bs);
            BS_DeSerializeFloat(&pData->heightPx, bs);
            pOutEnt->user.hData = hDebrisData;
            pOutEnt->init = &WfDebrisFieldEntityOnInit;
            pOutEnt->onDestroy = &WfDebrisFieldEntityOnDestroy;
            pOutEnt->bKeepInDynamicList = false;
            pOutEnt->bKeepInQuadtree = false;
            pOutEnt->bSerializeToDisk = false;
            pOutEnt->bSerializeToNetwork = false;
        }
        break;
    default:
        EASSERT(false);
        break;
    }
}

void WfSerializeDebrisFieldEntity(struct BinarySerializer* bs, struct Entity2D* pInEnt, struct GameLayer2DData* pData)
{
    EASSERT(false);
}