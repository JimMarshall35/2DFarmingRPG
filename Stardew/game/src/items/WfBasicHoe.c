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


#define HOE_TILE_DISTANCE_IN_FRONT_OF_PLAYER 32

struct ZZFXSound gThrustSnd = {1.0,0.05,64.799,0.023,0.129,0.395,0,1.0,0.0,-6.535,0.0,0.0,0.161,1.945,16.886331,0.464,0.229,0.461,0.156,0.0,-3441.073};

struct HoeUseContext
{
    struct GameFrameworkLayer* pLayer;
    struct GameLayer2DData* pData;
    HEntity2D hEntPlayer;
};

static OBJECT_POOL(struct HoeUseContext) gHoeUseContextPool;

static void InitHoeLookupTables()
{
    /*   ________
        |0_|1_|2_|
        |3_|__|4_|
        |5_|6_|7_|

         bit number
    */
    struct {
        const char* name;
        int numVals;
        u8 vals[4];
    } entries[256] = {
        { .name = "DryGround_BR_Bend",              .numVals = 1, .vals = { 0b01010000 } },
        { .name = "DryGround_Center",               .numVals = 1, .vals = { 0b00000000 } },
        { .name = "DryGround_B",                    .numVals = 1, .vals = { 0b01000000 } },
        { .name = "DryGround_BR",                   .numVals = 1, .vals = { 0b11010000 } },
        { .name = "DryGround_BLR",                  .numVals = 1, .vals = { 0b11111000 } },
        { .name = "DryGround_BL",                   .numVals = 1, .vals = { 0b01101000 } },
        { .name = "DryGround_CornerTuft_TL_TR",     .numVals = 1, .vals = { 0b11111010 } },
        { .name = "DryGround_CornerTuft_TR",        .numVals = 1, .vals = { 0b11111011 } },
        { .name = "DryGround_CornerTuft_TR_BR_BL",  .numVals = 1, .vals = { 0b01011011 } },
        { .name = "DryGround_R",                    .numVals = 1, .vals = { 0b00010000 } },
        { .name = "DryGround_LR",                   .numVals = 1, .vals = { 0b00011000 } },
        { .name = "DryGround_L",                    .numVals = 1, .vals = { 0b00001000 } },
        { .name = "DryGround_TB",                   .numVals = 1, .vals = { 0b01000010 } },
        { .name = "DryGround_BRT",                  .numVals = 1, .vals = { 0b00001000 } },
        { .name = "DryGround_BLRT",                 .numVals = 1, .vals = { 0b11111111 } },
        { .name = "DryGround_BLT",                  .numVals = 1, .vals = { 0b00010000 } },
        { .name = "DryGround_CornerTuft_TL_BL",     .numVals = 1, .vals = { 0b11011110 } },
        { .name = "DryGround_CornerTuft_TL",        .numVals = 1, .vals = { 0b11111110 } },
        { .name = "DryGround_CornerTuft_TL_TR_BR",  .numVals = 1, .vals = { 0b01111110 } },
        { .name = "DryGround_TR_Bend",              .numVals = 1, .vals = { 0b00010010 } },
        { .name = "DryGround_TL_Bend",              .numVals = 1, .vals = { 0b00001010 } },
        { .name = "DryGround_BL_Bend",              .numVals = 1, .vals = { 0b01001000 } },
        { .name = "DryGround_T",                    .numVals = 1, .vals = { 0b00000010 } },
        { .name = "DryGround_TLR",                  .numVals = 1, .vals = { 0b00000000 } },
        { .name = "DryGround_TR",                   .numVals = 1, .vals = { 0b00000000 } },
        { .name = "DryGround_TL",                   .numVals = 1, .vals = { 0b00000000 } },
        { .name = "DryGround_CornerTuft_BL_BR",     .numVals = 1, .vals = { 0b00000000 } },
        { .name = "DryGround_CornerTuft_BL",        .numVals = 1, .vals = { 0b00000000 } },
        { .name = "DryGround_CornerTuft_TL_TR_BL",  .numVals = 1, .vals = { 0b00000000 } },
        { .name = "DryGround_CornerTuft_TR_BR",     .numVals = 1, .vals = { 0b00000000 } },
        { .name = "DryGround_CornerTuft_BR",        .numVals = 1, .vals = { 0b00000000 } },
        { .name = "DryGround_CornerTuft_TL_BR_BL",  .numVals = 1, .vals = { 0b00000000 } },
    };
    
}

static bool ProcessHoeUsage(struct SDTimer* pTimer)
{
    struct HoeUseContext* pCtx = &gHoeUseContextPool[(HGeneric)pTimer->pUserData];

    struct Entity2D* pPlayerEnt = Et2D_GetEntity(&pCtx->pData->entities, pCtx->hEntPlayer);
    struct WfPlayerEntData* pPlayerData = WfGetPlayerEntData(pPlayerEnt);

    TileIndex* pIndex = WfGetTileInFrontOfPlayer(pCtx->pData, pPlayerData, HOE_TILE_DISTANCE_IN_FRONT_OF_PLAYER, pCtx->hEntPlayer, 0);
    if(pIndex)
    {
        TileIndex t = At_LookupNamedTile(pCtx->pData->hAtlas, "DryGround_Center");
        *pIndex = t;
    }

    FreeObjectPoolIndex(gHoeUseContextPool, (HGeneric)pTimer->pUserData);
    return true; /* remove timer */
}

static void OnMakeCurrentItem(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer)
{
    struct WfPlayerEntData* pEntData = WfGetPlayerEntData(pPlayer);
    pEntData->animationSet.layersMask = (1 << WfToolAnimationLayer);
    pEntData->animationSet.bgLayersMask = (1 << WfBG1);

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

static void OnStopBeingCurrentItem(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer)
{
    struct WfPlayerEntData* pEntData = WfGetPlayerEntData(pPlayer);
    pEntData->animationSet.layersMask &= ~(1 << WfToolAnimationLayer);
    struct Component2D* pComp = WfGetPlayerAnimationLayerComponent(pPlayer, WfToolAnimationLayer);
    pComp->data.spriteAnimator.bDraw = false;
}

static bool OnUseItem(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer)
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

static bool TryEquip(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer, enum WfEquipSlot slot)
{
    return false;
}


static struct WfItemDef gDef = 
{
    .UISpriteName = "basic-hoe",
    .pUserData = NULL,
    .onMakeCurrent = &OnMakeCurrentItem,
    .onStopBeingCurrent = &OnStopBeingCurrentItem,
    .onUseItem = &OnUseItem,
    .onTryEquip = &TryEquip,
    .onUseAnimation = WfThrustAnim,
    .bCanUseItem = true,
    .pickupSpriteName = "basic-hoe",
};

void WfAddBasicHoeDef()
{
    gHoeUseContextPool = NEW_OBJECT_POOL(struct HoeUseContext, 4);
    WfAddItemDef(&gDef);
}
