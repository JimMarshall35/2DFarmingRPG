#include "WfWoodedArea.h"
#include "BinarySerializer.h"
#include "Entities.h"
#include "GameFramework.h"
#include "AssertLib.h"
#include "ObjectPool.h"
#include "Game2DLayer.h"
#include "Physics2D.h"
#include "Random.h"
#include "WfEnums.h"
#include "Atlas.h"
#include <string.h>
#include "Components.h"
#include "WfTree.h"
#include "DrawContext.h"
#include "InputContext.h"
#include "Log.h"

struct WfWoodedAreaData
{
    float conif_percent, decid_percent, density, widthPx, heightPx;
};



static OBJECT_POOL(struct WfWoodedAreaData) gDataObjectPool;

void WfWoodedAreaInit()
{
    gDataObjectPool = NEW_OBJECT_POOL(struct WfWoodedAreaData, 8);
    WfTreeInit();
}


static void AddTreeAtRandomPos(float xMin, float xMax, float yMin, float yMax, struct WfTreeDef* def, struct GameLayer2DData* pLayerData)
{
    float xPos = Ra_FloatBetween(xMin, xMax);
    float yPos = Ra_FloatBetween(yMin, yMax);
    WfAddTreeBasedAt(xPos, yPos, def, pLayerData);
}

void WfWoodedAreaEntityOnInit(struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer, DrawContext* pDrawCtx, InputContext* pInputCtx)
{
    struct GameLayer2DData* pLayerData = pLayer->userData;

    /* spawn trees here */
    float pixelsPerMeter = Ph_GetPixelsPerMeter(pLayerData->hPhysicsWorld);
    struct WfWoodedAreaData* pData = &gDataObjectPool[pEnt->user.hData];
    float mWidth = pData->widthPx / pixelsPerMeter;
    float mHeight = pData->heightPx / pixelsPerMeter;
    float areaSquareMeters = mWidth * mHeight;
    int numTrees = (int)(areaSquareMeters * pData->density);
    Log_Verbose("Wooded Area Area: %.2f, Num Trees: %i", areaSquareMeters, numTrees);

    struct WfTreeDef treeDef;
    for(int i = 0; i < numTrees; i++)
    {
        treeDef.season = Summer;
        treeDef.type = Ra_RandZeroTo(2);
        treeDef.subtype = 0;
        AddTreeAtRandomPos(
            pEnt->transform.position[0], 
            pEnt->transform.position[0] + pData->widthPx, 
            pEnt->transform.position[1], 
            pEnt->transform.position[1] + pData->heightPx,
            &treeDef,
            pLayerData
        );
    }

    pLayerData->bCurrentLocationIsDirty = true;

    
    /* destroy the entity */
    Et2D_DestroyEntity(pLayer, &pLayerData->entities, pEnt->thisEntity);
}

void WfWoodedAreaEntityOnDestroy(struct Entity2D* pEnt, struct GameFrameworkLayer* pData)
{
    FreeObjectPoolIndex(gDataObjectPool, pEnt->user.hData);
}

void WfDeSerializeWoodedAreaEntityV1(struct BinarySerializer* bs, struct Entity2D* pOutEnt, struct GameLayer2DData* pData)
{
    int index = -1;
    gDataObjectPool = GetObjectPoolIndex(gDataObjectPool, &index);
    BS_DeSerializeFloat(&gDataObjectPool[index].conif_percent, bs);
    BS_DeSerializeFloat(&gDataObjectPool[index].decid_percent, bs);
    BS_DeSerializeFloat(&gDataObjectPool[index].density, bs);
    BS_DeSerializeFloat(&gDataObjectPool[index].widthPx, bs);
    BS_DeSerializeFloat(&gDataObjectPool[index].heightPx, bs);

    pOutEnt->user.hData = index;
    pOutEnt->init = &WfWoodedAreaEntityOnInit;
    pOutEnt->onDestroy = &WfWoodedAreaEntityOnDestroy;
    pOutEnt->bKeepInDynamicList = false;
    pOutEnt->bKeepInQuadtree = false;
    pOutEnt->bSerializeToDisk = false;
    pOutEnt->bSerializeToNetwork = false;
}

void WfDeSerializeWoodedAreaEntity(struct BinarySerializer* bs, struct Entity2D* pOutEnt, struct GameLayer2DData* pData)
{
    u32 version = 0;
    BS_DeSerializeU32(&version, bs);
    switch (version)
    {
    case 1:
        WfDeSerializeWoodedAreaEntityV1(bs, pOutEnt, pData);
        break;
    default:
        EASSERT(false);
        break;
    }
}

void WfSerializeWoodedAreaEntity(struct BinarySerializer* bs, struct Entity2D* pInEnt, struct GameLayer2DData* pData)
{
    EASSERT(false);
}