#include "WfDebris.h"
#include "BinarySerializer.h"
#include "Entities.h"
#include "Game2DLayer.h"
#include "WfGameLayerData.h"
#include "ObjectPool.h"
#include "Log.h"
#include "AssertLib.h"
#include "WfEntities.h"
#include "GameFramework.h"
#include "WfEntityMessages.h"


struct WfDebrisData
{
    int xTile, yTile;
    struct WfDebrisDef def;
};

static OBJECT_POOL(struct WfDebrisData) gDebrisDataPool = NULL;

static void RockOnDestroy(struct Entity2D* pEnt, struct GameFrameworkLayer* pData)
{
    FreeObjectPoolIndex(gDebrisDataPool, pEnt->user.hData);
    Entity2DOnDestroy(pEnt, pData);
}

static void DebrisHandleEntityMsg(struct Entity2D* pEnt, struct Entity2D* pSender, struct EntityToEntityMessage* pMsg, struct GameFrameworkLayer* pLayer)
{
    struct GameLayer2DData* pGameLayerData = pLayer->userData;
    struct WfDebrisData* pData = &gDebrisDataPool[pEnt->user.hData];
    switch(pMsg->type)
    {
    case E2EM_Damage:
        {
            struct WfDamageMsg* pDamageMessage = WfGetDamageMessage(pMsg);
            switch (pDamageMessage->type)
            {
            case WfPickaxeDamage:
                if(pData->def.type == WfRockType1 || pData->def.type == WfRockType2)
                {
                    pData->def.health -= pDamageMessage->damage;
                    if(pData->def.health <= 0)
                    {
                        Et2D_DestroyEntity(pLayer, &pGameLayerData->entities, pEnt->thisEntity);
                    }
                }
                break;
            case WfAxeDamage:
                if(pData->def.type == WfLogType1)
                {
                    pData->def.health -= pDamageMessage->damage;
                    if(pData->def.health <= 0)
                    {
                        Et2D_DestroyEntity(pLayer, &pGameLayerData->entities, pEnt->thisEntity);
                    }
                }
                break;
            }
        }
        break;
    };
}

void WfMakeEntityIntoDebrisBasedAt(struct Entity2D* pEnt, int xTile, int yTile, struct WfDebrisDef* def, struct GameLayer2DData* pGameLayerData)
{
    if(!gDebrisDataPool)
    {
        gDebrisDataPool = NEW_OBJECT_POOL(struct WfDebrisData, 64);
    }
    memset(pEnt, 0, sizeof(struct Entity2D));

    pEnt->transform.position[0] = xTile * 32.0f;
    pEnt->transform.position[1] = yTile * 32.0f;
    pEnt->transform.scale[0] = 1.0f;
    pEnt->transform.scale[1] = 1.0f;
    pEnt->transform.rotation = 0.0f;
    pEnt->transform.rotationPointRelative[0] = 0.0f;
    pEnt->transform.rotationPointRelative[1] = 0.0f;
    
    pEnt->bKeepInQuadtree = true;
    pEnt->bKeepInDynamicList = false;
    pEnt->type = WfEntityType_Rock;

    pEnt->numComponents = 0;
    struct Component2D* pComponent1 = &pEnt->components[pEnt->numComponents++];
    struct Component2D* pComponent2 = &pEnt->components[pEnt->numComponents++];

    
    struct WfSprites* pSprites = &((struct WfGameLayerData*)pGameLayerData->pUserData)->sprites;
    TileIndex sprites[3] = {pSprites->debrisSpritesPerSeason[Spring].rock2, pSprites->debrisSpritesPerSeason[Spring].rock1, pSprites->debrisSpritesPerSeason[Spring].debrisWood};

    pComponent1->type = ETE_Tiles;
    pComponent1->data.tiles.numTiles = 1;
    pComponent1->data.tiles.tiles[0].layer = 1;
    pComponent1->data.tiles.tiles[0].x = xTile;
    pComponent1->data.tiles.tiles[0].y = yTile;
    pComponent1->data.tiles.tiles[0].tile = sprites[def->type];

    pComponent2->type = ETE_StaticCollider;
    pComponent2->data.staticCollider.shape.type = PBT_Circle;
    pComponent2->data.staticCollider.shape.data.circle.center[0] = xTile * 32.0f + 16.0f;//transform2Ground[0];
    pComponent2->data.staticCollider.shape.data.circle.center[1] = yTile * 32.0f + 16.0f;//transform2Ground[1];
    pComponent2->data.staticCollider.shape.data.circle.radius = 16;
    pComponent2->data.staticCollider.bIsSensor = false;
    pComponent2->data.staticCollider.onSensorOverlapBegin = NULL;
    pComponent2->data.staticCollider.onSensorOverlapEnd = NULL;
    pComponent2->data.staticCollider.bGenerateSensorEvents = false;

    HGeneric hRockData = NULL_HANDLE;
    gDebrisDataPool = GetObjectPoolIndex(gDebrisDataPool, &hRockData);
    struct WfDebrisData* pRockData = &gDebrisDataPool[hRockData];
    pRockData->def.health = def->health;
    pRockData->def.type = def->type;
    pRockData->xTile = xTile;
    pRockData->yTile = yTile;

    pEnt->user.hData = hRockData;
    Et2D_PopulateCommonHandlers(pEnt);
    pEnt->onDestroy = &RockOnDestroy;
    pEnt->handleEntityMsg = &DebrisHandleEntityMsg;
    pEnt->bSerializeToDisk = true;
    pEnt->bSerializeToNetwork = true;

}

void WfDeSerializeDebrisEntity(struct BinarySerializer* bs, struct Entity2D* pOutEnt, struct GameLayer2DData* pData)
{
    i32 version = 0;
    BS_DeSerializeI32(&version, bs);
    switch (version)
    {
    case 1:
        {
            struct WfDebrisDef def;
            int xTile, yTile;
            BS_DeSerializeFloat(&def.health, bs);
            BS_DeSerializeI32(&def.type, bs);
            BS_DeSerializeI32(&xTile, bs);
            BS_DeSerializeI32(&yTile, bs);
            WfMakeEntityIntoDebrisBasedAt(pOutEnt, xTile, yTile, &def, pData);
        }
        break;
    default:
        Log_Error("unknown WfDeSerializeRockEntity version");
        EASSERT(false);
        break;
    }
}

void WfSerializeDebrisEntity(struct BinarySerializer* bs, struct Entity2D* pInEnt, struct GameLayer2DData* pData)
{
    struct WfDebrisData* pRockD = &gDebrisDataPool[pInEnt->user.hData];
    BS_SerializeI32(1, bs); // version
    BS_SerializeFloat(pRockD->def.health, bs);
    BS_SerializeI32(pRockD->def.type, bs);
    BS_SerializeI32(pRockD->xTile, bs);
    BS_SerializeI32(pRockD->yTile, bs);
}
