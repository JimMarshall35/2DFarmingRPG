#include "WfPlayerStart.h"
#include "Entities.h"
#include "BinarySerializer.h"
#include "Game2DLayer.h"
#include "ObjectPool.h"
#include "WfPlayer.h"
#include "GameFramework.h"
#include "WfWorld.h"
#include "string.h"
#include "Log.h"
#include "Network.h"
#include "WfGameLayerData.h"
#include "WfExit.h"
#include "EngineUtils.h"

static HEntity2D hCurrentLocalPlayer = NULL_HANDLE;

static OBJECT_POOL(struct WfPlayerStartData) gPlayerStartDataPool;

HEntity2D WfGetCurrentLocalPlayer()
{
    return hCurrentLocalPlayer;
}

void WfInitPlayerStart()
{
    gPlayerStartDataPool = NEW_OBJECT_POOL(struct WfPlayerStartData, 32);
}

void WfPlayerStartEntityOnInit(struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer, DrawContext* pDrawCtx, InputContext* pInputCtx)
{
    struct GameLayer2DData* pLayerData = pLayer->userData;
    Entity2DOnInit(pEnt, pLayer, pDrawCtx, pInputCtx);
    Log_Info("init playerstart entity");
    
    if(strcmp(WfWorld_GetCurrentLocationName(), gPlayerStartDataPool[pEnt->user.hData].from) == 0)
    {
        Log_Info("Spawning local player");
        struct WfGameLayerData* pWfData = pLayerData->pUserData;
        
        struct Entity2D ent;
        ent.user.hData = NULL_HANDLE;
        ent.nextSibling = NULL_HANDLE;
        ent.previousSibling = NULL_HANDLE;
        vec2 prevPos;
        WfGetPreviousAreaPosition(prevPos);
        vec2 spawnPos = {
            gPlayerStartDataPool[pEnt->user.hData].bUsePrevLocationX ? prevPos[0] : pEnt->transform.position[0],
            gPlayerStartDataPool[pEnt->user.hData].bUsePrevLocationY ? prevPos[1] : pEnt->transform.position[1],
        };
        WfMakeIntoPlayerEntity(&ent, pLayer, spawnPos);
        hCurrentLocalPlayer = Et2D_AddEntity(&pLayerData->entities, &ent);
        Log_Info("Local player entity ID %i, net id %i", hCurrentLocalPlayer, Et2D_GetEntity(&pLayerData->entities, hCurrentLocalPlayer)->networkID);
        WfWorld_SetCurrentLocationName(gPlayerStartDataPool[pEnt->user.hData].thisLocation);
    }
}

void WfPlayerStartEntityOnDestroy(struct Entity2D* pEnt, struct GameFrameworkLayer* pData)
{
    FreeObjectPoolIndex(gPlayerStartDataPool, pEnt->user.hData);
    Entity2DOnDestroy(pEnt, pData);
}

static void MakeEntityIntoPlayerStartEntity(struct Entity2D* pOutEnt, struct WfPlayerStartData* pData)
{
    gPlayerStartDataPool = GetObjectPoolIndex(gPlayerStartDataPool, &pOutEnt->user.hData);

    memcpy(&gPlayerStartDataPool[pOutEnt->user.hData], pData, sizeof(struct WfPlayerStartData));
    HGeneric hPlayer = pOutEnt->user.hData;
    pOutEnt->init = &WfPlayerStartEntityOnInit;
    pOutEnt->bKeepInDynamicList = false;
    pOutEnt->bKeepInQuadtree = false;
    pOutEnt->bSerializeToDisk = true;
    pOutEnt->bSerializeToNetwork = true;
}

void WfDeSerializePlayerStartEntityV1(struct BinarySerializer* bs, struct Entity2D* pOutEnt, struct GameLayer2DData* pData)
{
    struct WfPlayerStartData data;
    ZeroMemory(&data, sizeof(struct WfPlayerStartData));
    BS_DeSerializeStringInto(data.from, bs);
    BS_DeSerializeStringInto(data.thisLocation, bs);
    i32 i;
    BS_DeSerializeI32(&i, bs);
    data.bUsePrevLocationX = (i != 0);
    BS_DeSerializeI32(&i, bs);
    data.bUsePrevLocationY = (i != 0);
    MakeEntityIntoPlayerStartEntity(pOutEnt, &data);
}

void WfDeSerializePlayerStartEntity(struct BinarySerializer* bs, struct Entity2D* pOutEnt, struct GameLayer2DData* pData)
{
    u32 version;
    BS_DeSerializeU32(&version, bs);
    switch (version)
    {
    case 1:
        /* code */
        WfDeSerializePlayerStartEntityV1(bs, pOutEnt, pData);
        break;   
    default:
        break;
    }
}

void WfSerializePlayerStartEntity(struct BinarySerializer* bs, struct Entity2D* pInEnt, struct GameLayer2DData* pData)
{
    BS_SerializeU32(1, bs); // version
    BS_SerializeString(gPlayerStartDataPool[pInEnt->user.hData].from, bs);
    BS_SerializeString(gPlayerStartDataPool[pInEnt->user.hData].thisLocation, bs);

    BS_SerializeI32((i32)gPlayerStartDataPool[pInEnt->user.hData].bUsePrevLocationX, bs);
    BS_SerializeI32((i32)gPlayerStartDataPool[pInEnt->user.hData].bUsePrevLocationY, bs);
}

HEntity2D WfAddPlayerStartEntityAt(struct Entity2DCollection* pEntities, struct WfPlayerStartData* pData, float x, float y)
{
    struct Entity2D ent;
    ZeroMemory(&ent, sizeof(struct Entity2D));
    ent.transform.position[0] = x;
    ent.transform.position[1] = y;
    Et2D_PopulateCommonHandlers(&ent);
    MakeEntityIntoPlayerStartEntity(&ent, pData);
    return Et2D_AddEntity(pEntities, &ent);

}