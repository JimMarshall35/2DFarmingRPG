#include "WfDebrisField.h"
#include "Entities.h"
#include "Game2DLayer.h"
#include "ObjectPool.h"
#include "BinarySerializer.h"
#include "AssertLib.h"
#include "GameFramework.h"
#include "WfItemHelpers.h"
#include "WfDebris.h"
#include "WfGameLayerData.h"
#include "Random.h"
#include "Log.h"

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

static void RandomTilePlacement(int tlX, int tlY, int w, int h, int* x, int* y)
{
    int xIncr = Ra_RandZeroTo(w);
    int yIncr = Ra_RandZeroTo(h);
    *x = tlX + xIncr;
    *y = tlY + yIncr;
}

static bool IsDebrisAt(int x, int y, struct TileMap* pMap, struct WfSprites* pSprites)
{
    bool b = false;
    TileIndex pIndex = *WfGetTileAtXY(&pMap->layers[1], x, y);
    b = pIndex == pSprites->debrisSpritesPerSeason[Spring].rock1 ||
        pIndex == pSprites->debrisSpritesPerSeason[Spring].rock2 ||
        pIndex == pSprites->debrisSpritesPerSeason[Spring].debrisWood;
    return b;
}

static enum WfDebrisType GetDebrisType(struct WfDebrisFieldData* pData)
{
    float f = Ra_FloatBetween(0, 1);
    if(f < pData->LogsPercentage)
    {
        return WfLogType1;
    }
    else
    {
        if(Ra_FloatBetween(0, 1) >= 0.5)
        {
            return WfRockType1;
        }
        else
        {
            return WfRockType2;
        }
    }
    return WfRockType1;
}

void WfDebrisFieldEntityOnInit(struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer, DrawContext* pDrawCtx, InputContext* pInputCtx)
{
    struct GameLayer2DData* pLayerData = pLayer->userData;
    struct WfSprites* pSprites = &((struct WfGameLayerData*)pLayerData->pUserData)->sprites;

    struct WfDebrisFieldData* pData = &gDebrisFieldPool[pEnt->user.hData];

    float areaPX = pData->widthPx * pData->heightPx;
    float areaTiles = areaPX / (32.0f * 32.0f);
    int numTiles = (int)(areaTiles * pData->Density);

    Log_Info("Debris field num tiles: %i", numTiles);

    //pLayerData->tilemap.layers[1].
    int tileTLX = pEnt->transform.position[0] / 32.0f;
    int tileTLY = pEnt->transform.position[1] / 32.0f;

    for(int i=0; i<numTiles; i++)
    {
        int x, y;
        do
        {
            RandomTilePlacement(tileTLX, tileTLY, pData->widthPx / 32, pData->heightPx / 32, &x, &y);
        } while (IsDebrisAt(x, y, &pLayerData->tilemap, pSprites));
        
        struct Entity2D ent;
        struct WfDebrisDef def = {
            .health = 30,
            .type = GetDebrisType(pData)
        };
        WfMakeEntityIntoDebrisBasedAt(&ent, x, y, &def, pLayerData);
        Et2D_AddEntity(&pLayerData->entities, &ent);
    }

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