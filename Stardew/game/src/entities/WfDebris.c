#include "WfDebris.h"
#include <string.h>
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
#include "WfTileLayers.h"

struct ZZFXSound gWrongDamageTypeSFX = {1.0,0.05,55.748,0.066,0.252,0.447,0,1.0,0.0,0.0,0.0,0.0,0.062,1.633,0.0,0.333,0.433,0.309,0.171,0.057,0.0};
struct ZZFXSound gPickaxeHitRockSFX = {1.0,0.05,62.25,0.011,0.133,0.298,3,1.0,1.503,0.0,0.0,0.0,0.29,1.191,0.0,0.619,0.0,0.485,0.2,0.0,-2357.049};
struct ZZFXSound gDestroyRockSFX = {1.0,0.05,88.199,0.028,0.125,0.42,5,1.0,0.0,4.522,0.0,0.0,0.0,0.359,6.641019,0.816,0.0,0.423,0.166,0.0,0.0};

struct ZZFXSound gLogHitSFX = {1.0,0.05,63.14,0.049,0.086,0.203,2,1.0,3.678,-4.693,0.0,0.0,0.261,0.185,0.0,0.699,0.0,0.388,0.108,0.087,0.0};
struct ZZFXSound gLogDestroySFX = {1.0,0.05,590.6,0.005,0.28,0.257,1,1.0,0.0,0.0,-196.86,0.159,0.165,0.12,15.624050000000004,0.168,0.0,0.635,0.227,0.421,0.0};

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
                        pGameLayerData->bCurrentLocationIsDirty = true;
                        Au_PlayZzFX(&gDestroyRockSFX);
                    }
                    else
                    {
                        Au_PlayZzFX(&gPickaxeHitRockSFX);
                    }
                }
                else
                {
                    Au_PlayZzFX(&gWrongDamageTypeSFX);
                }
                break;
            case WfAxeDamage:
                if(pData->def.type == WfLogType1)
                {
                    pData->def.health -= pDamageMessage->damage;
                    if(pData->def.health <= 0)
                    {
                        Et2D_DestroyEntity(pLayer, &pGameLayerData->entities, pEnt->thisEntity);
                        pGameLayerData->bCurrentLocationIsDirty = true;
                        Au_PlayZzFX(&gLogDestroySFX);
                    }
                    else
                    {
                        Au_PlayZzFX(&gLogHitSFX);
                    }
                }
                else
                {
                    Au_PlayZzFX(&gWrongDamageTypeSFX);
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
    TileIndex sprites[3] = {pSprites->debrisSprites.rock2, pSprites->debrisSprites.rock1, pSprites->debrisSprites.debrisWood};

    pComponent1->type = ETE_Tiles;
    pComponent1->data.tiles.numTiles = 1;
    pComponent1->data.tiles.tiles[0].layer = WFLAYER_GROUND2;
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
    pEnt->bKeepInQuadtree = true;
    pEnt->bKeepInDynamicList = true;

}

HEntity2D WfAddDebrisBasedAt(float x, float y, struct WfDebrisDef* def, struct GameLayer2DData* pGameLayerData)
{
    struct Entity2D ent;
    WfMakeEntityIntoDebrisBasedAt(&ent, x, y, def, pGameLayerData);
    return Et2D_AddEntity(&pGameLayerData->entities, &ent);
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
            BS_DeSerializeI32((i32*)&def.type, bs);
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
