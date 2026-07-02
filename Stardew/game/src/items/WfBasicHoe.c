#include "WfBasicHoe.h"
#include <stdlib.h>
#include "WfItem.h"
#include "WfItem.h"
#include "WfPlayer.h"
#include "Entities.h"
#include "EngineUtils.h"
#include "Audio.h"
#include "ObjectPool.h"
#include "GameFramework.h"
#include "WfItemHelpers.h"
#include "Atlas.h"
#include "IntTypes.h"
#include "Log.h"
#include "WfDryGroundLUT.h"
#include "WfTileLayers.h"

#define HOE_TILE_DISTANCE_IN_FRONT_OF_PLAYER 32

struct ZZFXSound gThrustSnd = {1.0,0.05,64.799,0.023,0.129,0.395,0,1.0,0.0,-6.535,0.0,0.0,0.161,1.945,16.886331,0.464,0.229,0.461,0.156,0.0,-3441.073};


struct HoeUseContext
{
    struct GameFrameworkLayer* pLayer;
    struct GameLayer2DData* pData;
    HEntity2D hEntPlayer;
};

static OBJECT_POOL(struct HoeUseContext) gHoeUseContextPool = NULL;

static VECTOR(TileIndex) gTilledTileIndices = NULL;

#define NUM_GRASS_TILES 3
TileIndex gGrassTileIndices[NUM_GRASS_TILES] = {
    0,0,0
};

static bool TileVectorContains(TileIndex t)
{
    for(int i=0; i<VectorSize(gTilledTileIndices); i++)
    {
        if(gTilledTileIndices[i] == t)
        {
            return true;
        }
    }
    return false;
}

static void SetTileTilled(int x, int y, TileIndex* pIndex, struct TileMap* pTM, hAtlas atlas)
{
    u8 index = WfGetTerrainLUTIndex(x, y, pTM, 0, gTilledTileIndices, VectorSize(gTilledTileIndices));
    Log_Info("\n%i%i%i\n%i_%i\n%i%i%i\n",
        (index & (1 << 0)) == 0 ? 0 : 1,
        (index & (1 << 1)) == 0 ? 0 : 1,
        (index & (1 << 2)) == 0 ? 0 : 1,
        (index & (1 << 3)) == 0 ? 0 : 1,
        (index & (1 << 4)) == 0 ? 0 : 1,
        (index & (1 << 5)) == 0 ? 0 : 1,
        (index & (1 << 6)) == 0 ? 0 : 1,
        (index & (1 << 7)) == 0 ? 0 : 1
    );
    TileIndex t = At_LookupNamedTile(atlas, gDryGroundLUT[index]);
    *pIndex = t;

    pIndex = WfGetTileAtXY(&pTM->layers[0], x - 1, y -1);
    t = *pIndex;
    if(TileVectorContains(t))
    {
        u8 index = WfGetTerrainLUTIndex(x - 1, y - 1, pTM, 0, gTilledTileIndices, VectorSize(gTilledTileIndices));

        TileIndex t = At_LookupNamedTile(atlas, gDryGroundLUT[index]);
        *pIndex = t;
    }

    pIndex = WfGetTileAtXY(&pTM->layers[0], x, y -1);
    t = *pIndex;
    if(TileVectorContains(t))
    {
        u8 index = WfGetTerrainLUTIndex(x, y - 1, pTM, 0, gTilledTileIndices, VectorSize(gTilledTileIndices));
        TileIndex t = At_LookupNamedTile(atlas, gDryGroundLUT[index]);
        *pIndex = t;
    }

    pIndex = WfGetTileAtXY(&pTM->layers[0], x + 1, y - 1);
    t = *pIndex;
    if(TileVectorContains(t))
    {
        u8 index = WfGetTerrainLUTIndex(x + 1, y - 1, pTM, 0, gTilledTileIndices, VectorSize(gTilledTileIndices));
        TileIndex t = At_LookupNamedTile(atlas, gDryGroundLUT[index]);
        *pIndex = t;
    }

    pIndex = WfGetTileAtXY(&pTM->layers[0], x - 1, y);
    t = *pIndex;
    if(TileVectorContains(t))
    {
        u8 index = WfGetTerrainLUTIndex(x - 1, y, pTM, 0, gTilledTileIndices, VectorSize(gTilledTileIndices));
        TileIndex t = At_LookupNamedTile(atlas, gDryGroundLUT[index]);
        *pIndex = t;
    }

    pIndex = WfGetTileAtXY(&pTM->layers[0], x + 1, y);
    t = *pIndex;
    if(TileVectorContains(t))
    {
        u8 index = WfGetTerrainLUTIndex(x + 1, y, pTM, 0, gTilledTileIndices, VectorSize(gTilledTileIndices));
        TileIndex t = At_LookupNamedTile(atlas, gDryGroundLUT[index]);
        *pIndex = t;
    }

    pIndex = WfGetTileAtXY(&pTM->layers[0], x - 1, y + 1);
    t = *pIndex;
    if(TileVectorContains(t))
    {
        u8 index = WfGetTerrainLUTIndex(x - 1, y + 1, pTM, 0, gTilledTileIndices, VectorSize(gTilledTileIndices));
        TileIndex t = At_LookupNamedTile(atlas, gDryGroundLUT[index]);
        *pIndex = t;
    }

    pIndex = WfGetTileAtXY(&pTM->layers[0], x, y + 1);
    t = *pIndex;
    if(TileVectorContains(t))
    {
        u8 index = WfGetTerrainLUTIndex(x, y + 1, pTM, 0, gTilledTileIndices, VectorSize(gTilledTileIndices));
        TileIndex t = At_LookupNamedTile(atlas, gDryGroundLUT[index]);
        *pIndex = t;
    }

    pIndex = WfGetTileAtXY(&pTM->layers[0], x + 1, y + 1);
    t = *pIndex;
    if(TileVectorContains(t))
    {
        u8 index = WfGetTerrainLUTIndex(x + 1, y + 1, pTM, 0, gTilledTileIndices, VectorSize(gTilledTileIndices));
        TileIndex t = At_LookupNamedTile(atlas, gDryGroundLUT[index]);
        *pIndex = t;
    }
}

static bool ProcessHoeUsage(struct SDTimer* pTimer)
{
    struct HoeUseContext* pCtx = &gHoeUseContextPool[(HGeneric)pTimer->pUserData];

    struct Entity2D* pPlayerEnt = Et2D_GetEntity(&pCtx->pData->entities, pCtx->hEntPlayer);
    struct WfPlayerEntData* pPlayerData = WfGetPlayerEntData(pPlayerEnt);

    int x, y;
    TileIndex* pIndex = WfGetTileInFrontOfPlayer(
        pCtx->pData, pPlayerData, HOE_TILE_DISTANCE_IN_FRONT_OF_PLAYER, pCtx->hEntPlayer, WFLAYER_GROUND, &x, &y);

    bool bIsTileAllowed = false;
    for(int i=0; i<NUM_GRASS_TILES; i++)
    {
        /* don't allow to be used on any ground layer tile other than ones that form gGrassTileIndices */
        if(gGrassTileIndices[i] == *pIndex)
        {
            bIsTileAllowed = true;
            break;
        }
    }
    if(bIsTileAllowed)
    {
        /* don't allow to be used if there is debris on top of the ground layer */
        TileIndex* pIndex = WfGetTileAtXY(&pCtx->pData->tilemap.layers[WFLAYER_GROUND2], x, y);
        if(*pIndex)
        {
            bIsTileAllowed = false;
        }
    }
    if(!bIsTileAllowed)
    {
        return true;
    }

    if(pIndex)
    {
        SetTileTilled(x, y, pIndex, &pCtx->pData->tilemap, pCtx->pData->hAtlas);
    }

    FreeObjectPoolIndex(gHoeUseContextPool, (HGeneric)pTimer->pUserData);
    return true; /* remove timer */
}

void WfBasicHoeOnMakeCurrentItem(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer)
{
    struct WfPlayerEntData* pEntData = WfGetPlayerEntData(pPlayer);
    pEntData->animationSet.layersMask |= (1 << WfToolAnimationLayer);
    pEntData->animationSet.bgLayersMask |= (1 << WfBG1);

    pEntData->animationSet.layers[WfToolAnimationLayer].walkAnimations[Up] = "walk-hoe-male-up";
    pEntData->animationSet.layers[WfToolAnimationLayer].walkAnimations[Down] = "walk-hoe-male-down";
    pEntData->animationSet.layers[WfToolAnimationLayer].walkAnimations[Left] = "walk-hoe-male-left";
    pEntData->animationSet.layers[WfToolAnimationLayer].walkAnimations[Right] = "walk-hoe-male-right";

    pEntData->animationSet.layers[WfToolAnimationLayer].thrustAnimations[Up] = "hoe-thrust-male-up-fg";
    pEntData->animationSet.layers[WfToolAnimationLayer].thrustAnimations[Down] = "hoe-thrust-male-down-fg";
    pEntData->animationSet.layers[WfToolAnimationLayer].thrustAnimations[Left] = "hoe-thrust-male-left-fg";
    pEntData->animationSet.layers[WfToolAnimationLayer].thrustAnimations[Right] = "hoe-thrust-male-right-fg";

    ZeroMemory(pEntData->animationSet.bgLayers[WfBG1].walkAnimations, sizeof(const char*) * NUM_ANIMATIONS);
    pEntData->animationSet.bgLayers[WfBG1].thrustAnimations[Up] = "hoe-thrust-male-up-bg";
    pEntData->animationSet.bgLayers[WfBG1].thrustAnimations[Down] = "hoe-thrust-male-down-bg";
    pEntData->animationSet.bgLayers[WfBG1].thrustAnimations[Left] = "hoe-thrust-male-left-bg";
    pEntData->animationSet.bgLayers[WfBG1].thrustAnimations[Right] = "hoe-thrust-male-right-bg";


    struct Component2D* pComp = WfGetPlayerAnimationLayerComponent(pPlayer, WfToolAnimationLayer);
    WfSetPlayerOverlayAnimations(pEntData->directionFacing, pLayer, pEntData, pPlayer);
    pComp->data.spriteAnimator.onSprite = 0;
}

void WfBasicHoeOnGameLayerPush(struct WfItemDef* pDef, struct GameFrameworkLayer* pLayer, DrawContext* drawContext, InputContext* inputContext)
{
    if(gHoeUseContextPool == NULL)
    {
        gHoeUseContextPool = NEW_OBJECT_POOL(struct HoeUseContext, 4);
    }
    if(gTilledTileIndices == NULL)
    {
        gTilledTileIndices = NEW_VECTOR(TileIndex);
        gTilledTileIndices = VectorResize(gTilledTileIndices, DryGroundLUT_NamesLen);
    }
    struct GameLayer2DData* pEngineLayer = pLayer->userData;
    gTilledTileIndices = VectorClear(gTilledTileIndices);
    for(int i=0; i<DryGroundLUT_NamesLen; i++)
    {
        TileIndex t = At_LookupNamedTile(pEngineLayer->hAtlas, gDryGroundLUT_Names[i]);
        gTilledTileIndices = VectorPush(gTilledTileIndices, &t);
    }
    gGrassTileIndices[0] = At_LookupNamedTile(pEngineLayer->hAtlas, "grass_1");
    gGrassTileIndices[1] = At_LookupNamedTile(pEngineLayer->hAtlas, "grass_2");
    gGrassTileIndices[2] = At_LookupNamedTile(pEngineLayer->hAtlas, "grass_3");
}

void WfBasicHoeOnGameLayerPop(struct WfItemDef* pDef, struct GameFrameworkLayer* pLayer, DrawContext* drawContext, InputContext* inputContext)
{

}

void WfBasicHoeOnStopBeingCurrentItem(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer)
{
    struct WfPlayerEntData* pEntData = WfGetPlayerEntData(pPlayer);
    pEntData->animationSet.layersMask &= ~(1 << WfToolAnimationLayer);
    struct Component2D* pComp = WfGetPlayerAnimationLayerComponent(pPlayer, WfToolAnimationLayer);
    pComp->data.spriteAnimator.bDraw = false;
}

bool WfBasicHoeOnUseItem(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer)
{
    Au_PlayZzFX(&gThrustSnd);
    struct GameLayer2DData* pData = pLayer->userData;

    HGeneric hCtx = NULL_HANDLE;
    gHoeUseContextPool = GetObjectPoolIndex(gHoeUseContextPool, &hCtx);
    
    struct HoeUseContext ctx = {
        .hEntPlayer = pPlayer->thisEntity,
        .pData = pData,
        .pLayer = pLayer
    };
    gHoeUseContextPool[hCtx] = ctx;
    struct WfPlayerEntData* pEntData = WfGetPlayerEntData(pPlayer);
    char* animName = pEntData->animationSet.layers[WfToolAnimationLayer].thrustAnimations[pEntData->directionFacing];
    HTimer t = WfScheduleCallbackOnAnimation(pPlayer, pLayer, &ProcessHoeUsage, 0.3, animName, (void*)hCtx);
    pData->bCurrentLocationIsDirty = true;
    return true;
}
