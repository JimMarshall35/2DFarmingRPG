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
#include "ZzFX.h"
#include "Audio.h"

struct ZZFXSound gPickaxeHitRockSFX = {1.0,0.05,60.562,0.072,0.028,0.207,1,1.0,0.0,0.0,0.0,0.0,0.238,0.223,0.0,0.264,0.001,0.482,0.177,0.223,-3087.834};
struct ZZFXSound gDestroyRockSFX = {1.0,0.05,80.562,0.038,0.257,0.308,5,1.0,-4.614,0.0,0.0,0.0,0.237,1.446,0.0,0.926,0.0,0.461,0.057,0.0,0.0};

struct WfDebrisData
{
    int xTile, yTile;
    struct WfDebrisDef def;
    vec2 groundContactPoint;
};

static OBJECT_POOL(struct WfDebrisData) gDebrisDataPool = NULL;

static void RockOnDestroy(struct Entity2D* pEnt, struct GameFrameworkLayer* pData)
{
    FreeObjectPoolIndex(gDebrisDataPool, pEnt->user.hData);
    Entity2DOnDestroy(pEnt, pData);
}

void WfDebrisGetGroundContactPoint(struct Entity2D* pEnt, vec2 outPoint)
{
    struct WfDebrisData* pData = &gDebrisDataPool[pEnt->user.hData];
    outPoint[0] = pData->groundContactPoint[0];
    outPoint[1] = pData->groundContactPoint[1];
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

    pRockData->groundContactPoint[0] = pEnt->transform.position[0];
    pRockData->groundContactPoint[1] = pEnt->transform.position[1];
    struct WfDebrisData* pData = &gDebrisDataPool[pEnt->user.hData];

    struct TileMap* pTM = &pGameLayerData->tilemap;
    
    vec2 addition = {
        pTM->layers[pEnt->components[0].data.tiles.tiles[0].layer].tileWidthPx,
        pTM->layers[pEnt->components[0].data.tiles.tiles[0].layer].tileHeightPx
    };
    glm_vec2_add(pRockData->groundContactPoint, addition, pRockData->groundContactPoint);

    pEnt->user.hData = hRockData;
    Et2D_PopulateCommonHandlers(pEnt);
    pEnt->onDestroy = &RockOnDestroy;
    pEnt->handleEntityMsg = &DebrisHandleEntityMsg;
    pEnt->bSerializeToDisk = true;
    pEnt->bSerializeToNetwork = true; 

    // TODO: Put in the quad tree instead - the quad tree is fucked though.
    // re-write the quad tree as a BSP tree as detailed in "Game design patterns"
    pEnt->bKeepInQuadtree = false;
    pEnt->bKeepInDynamicList = true;

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
