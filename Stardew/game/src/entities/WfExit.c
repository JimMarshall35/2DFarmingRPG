#include "WfExit.h"
#include "Entities.h"
#include "GameFramework.h"
#include "Game2DLayer.h"
#include "BinarySerializer.h"
#include "ObjectPool.h"
#include "Components.h"
#include "Entities.h"
#include "WfEntities.h"
#include "WfWorld.h"
#include "WfHUD.h"

struct WfExitEntityData
{
    /* name of area this exit leads to */
    char toArea[64];
    float w, h;
};

static OBJECT_POOL(struct WfExitEntityData) gExitEntityDataPool = NULL;

static void DestroyExitEntity(struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer)
{
    FreeObjectPoolIndex(gExitEntityDataPool, pEnt->user.hData);
    Entity2DOnDestroy(pEnt, pLayer);
}

void WfInitExit()
{
    gExitEntityDataPool = NEW_OBJECT_POOL(struct WfExitEntityData, 16);
}

void WfOnExitSensorOverlapBegin(struct GameFrameworkLayer* pLayer, HEntity2D hOverlappingEntity, HEntity2D thisSensorEntity)
{
    struct GameLayer2DData* pLayerData = pLayer->userData;
    struct Entity2D* pSensorEnt = Et2D_GetEntity(&pLayerData->entities, thisSensorEntity);
    struct Entity2D* pOverlappingEnt = Et2D_GetEntity(&pLayerData->entities, hOverlappingEntity);
    if(pOverlappingEnt->type == WfEntityType_Player)
    {
        struct WfExitEntityData* pSensorData = &gExitEntityDataPool[pSensorEnt->user.hData];
        /* TODO: add a bDirty flag on the GameLayer2DData indicating whether we need to save the level here */
        GF_PopGameFrameworkLayer();
        GF_PopGameFrameworkLayer();
        WfWorld_LoadLocation(pSensorData->toArea, pLayerData->pDrawContext);
        WfPushHUD(pLayerData->pDrawContext);
    }
}

void WfDeSerializeExitEntityV1(struct BinarySerializer* bs, struct Entity2D* pOutEnt, struct GameLayer2DData* pData)
{
    HGeneric hExitData = NULL_HANDLE;
    gExitEntityDataPool = GetObjectPoolIndex(gExitEntityDataPool, &hExitData);
    pOutEnt->user.hData = hExitData;
    BS_DeSerializeFloat(&gExitEntityDataPool[hExitData].w, bs);
    BS_DeSerializeFloat(&gExitEntityDataPool[hExitData].h, bs);
    BS_DeSerializeStringInto(gExitEntityDataPool[hExitData].toArea, bs);
    Et2D_PopulateCommonHandlers(pOutEnt);
    pOutEnt->onDestroy = &DestroyExitEntity;
    struct Component2D* pComponent1 = &pOutEnt->components[pOutEnt->numComponents++];
    pComponent1->type = ETE_StaticCollider;
    pComponent1->data.staticCollider.bIsSensor = true;
    pComponent1->data.staticCollider.onSensorOverlapBegin = &WfOnExitSensorOverlapBegin;
    pComponent1->data.staticCollider.onSensorOverlapEnd = NULL;
    pComponent1->data.staticCollider.shape.type = PBT_Rect;
    pComponent1->data.staticCollider.shape.data.rect.w = gExitEntityDataPool[hExitData].w;
    pComponent1->data.staticCollider.shape.data.rect.h = gExitEntityDataPool[hExitData].h;
    pComponent1->data.staticCollider.bGenerateSensorEvents = true;
}

void WfDeSerializeExitEntity(struct BinarySerializer* bs, struct Entity2D* pOutEnt, struct GameLayer2DData* pData)
{
    u32 version;
    BS_DeSerializeU32(&version, bs);
    switch (version)
    {
    case 1:
        /* code */
        WfDeSerializeExitEntityV1(bs, pOutEnt, pData);
        break;
    default:
        break;
    }
}

void WfSerializeExitEntity(struct BinarySerializer* bs, struct Entity2D* pInEnt, struct GameLayer2DData* pData)
{
    BS_SerializeU32(1, bs);
    struct WfExitEntityData* pEntData = &gExitEntityDataPool[pInEnt->user.hData];
    BS_SerializeFloat(pEntData->w, bs);
    BS_SerializeFloat(pEntData->h, bs);
}