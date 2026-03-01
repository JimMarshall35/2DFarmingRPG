#include "WfBasicAxe.h"
#include "WfItem.h"
#include "WfPlayer.h"
#include "Entities.h"

#include <stdlib.h>
#include "EngineUtils.h"
#include "Audio.h"

#include "TimerPool.h"
#include "Game2DLayer.h"
#include "GameFramework.h"
#include "ObjectPool.h"
#include "WfItemHelpers.h"
#include "EntityQuadTree.h"
#include "Game2DLayer.h"
#include "Geometry.h"
#include "Maths.h"
#include "WfEntityMessages.h"
#include "Log.h"

#define AXE_FAN_WIDTH DEGREES_TO_RADIANS(45.0f)

#define AXE_FAN_LENGTH (64.0F)

#define AXE_DAMAGE 10.0f

struct AxeHitHandlerContext
{
    struct GameFrameworkLayer* pLayer;
    struct GameLayer2DData* pData;
    HEntity2D hEntPlayer;
};

static OBJECT_POOL(struct AxeHitHandlerContext) gAxeHitContextPool;

static struct ZZFXSound gSwingSound = {1.0,0.05,32.268,0.008,0.046,0.208,0,1.0,2.662,2.312,0.0,0.0,0.178,1.778,0.0,0.106,0.0,0.431,0.174,0.076,0.0};

static void OnMakeCurrentItem(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer)
{
    struct WfPlayerEntData* pEntData = WfGetPlayerEntData(pPlayer);
    pEntData->animationSet.layersMask = (1 << WfToolAnimationLayer);
    pEntData->animationSet.bgLayersMask = (1 << WfBG1);
    
    pEntData->animationSet.layers[WfToolAnimationLayer].walkAnimations[Up] = "walk-axe-male-up";
    pEntData->animationSet.layers[WfToolAnimationLayer].walkAnimations[Down] = "walk-axe-male-down";
    pEntData->animationSet.layers[WfToolAnimationLayer].walkAnimations[Left] = "walk-axe-male-left";
    pEntData->animationSet.layers[WfToolAnimationLayer].walkAnimations[Right] = "walk-axe-male-right";

    pEntData->animationSet.layers[WfToolAnimationLayer].slashAnimations[Up] = "axe-slash-male-up-fg";
    pEntData->animationSet.layers[WfToolAnimationLayer].slashAnimations[Down] = "axe-slash-male-down-fg";
    pEntData->animationSet.layers[WfToolAnimationLayer].slashAnimations[Left] = "axe-slash-male-left-fg";
    pEntData->animationSet.layers[WfToolAnimationLayer].slashAnimations[Right] = "axe-slash-male-right-fg";

    ZeroMemory(pEntData->animationSet.bgLayers[WfBG1].walkAnimations, sizeof(const char*) * NUM_ANIMATIONS);
    pEntData->animationSet.bgLayers[WfBG1].slashAnimations[Up] = "axe-slash-male-up-bg";
    pEntData->animationSet.bgLayers[WfBG1].slashAnimations[Down] = "axe-slash-male-down-bg";
    pEntData->animationSet.bgLayers[WfBG1].slashAnimations[Left] = "axe-slash-male-left-bg";
    pEntData->animationSet.bgLayers[WfBG1].slashAnimations[Right] = "axe-slash-male-right-bg";


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



static bool ProcessAxeUsage(struct SDTimer* pTimer)
{
    struct AxeHitHandlerContext* pCtx = &gAxeHitContextPool[(HGeneric)pTimer->pUserData];

    struct Entity2D* pPlayerEnt = Et2D_GetEntity(&pCtx->pData->entities, pCtx->hEntPlayer);
    struct WfPlayerEntData* pPlayerData = WfGetPlayerEntData(pPlayerEnt);
    vec2 playerGroundPos;
    WfPlayerGetGroundContactPoint(pPlayerEnt, playerGroundPos);

    vec2 dir;
    WfGetDirectionVector(pPlayerData->directionFacing, dir);
    struct WfSearchFan fan = 
    {
        .base[0] = playerGroundPos[0],
        .base[1] = playerGroundPos[1],
        .direction[0] = dir[0],
        .direction[1] = dir[1], 
        .length = AXE_FAN_LENGTH,
        .widthRadians = AXE_FAN_WIDTH,
    };
    static VECTOR(struct Entity2D*) sFoundEnts = NULL;
	if(!sFoundEnts)
	{
		sFoundEnts = NEW_VECTOR(struct Entity2D*);
	}

    sFoundEnts = VectorClear(sFoundEnts);
    
    sFoundEnts = WfFindEntitiesWithinFan(&fan, pCtx->pLayer, pCtx->pData, WfDynamicEntities | WfStaticEntities, sFoundEnts);
    
    Log_Info("Entities in fan %i", VectorSize(sFoundEnts));

    struct Entity2D* pHit = VectorSize(sFoundEnts) > 0 ? sFoundEnts[0] : NULL;//WfFindClosestEntity(fan.base, sFoundEnts);

    if(pHit)
    {
        struct WfDamageMsg* pDamageMsg = NULL;

        struct EntityToEntityMessage msg = 
        {
            .type = E2EM_Damage,
            .data.hMsgData = WfAllocateDamageMessageData(&pDamageMsg),
            .freer = &WfDamageMsgFreer,
            .sender = pPlayerEnt->thisEntity,
            .recipient = pHit->thisEntity
        };
        pDamageMsg->damage = AXE_DAMAGE;
        pDamageMsg->type = WfAxeDamage;
        Et2D_SendEntity2EntityMsg(&pCtx->pData->entities, &msg);
    }

    FreeObjectPoolIndex(gAxeHitContextPool, (HGeneric)pTimer->pUserData);
    return true; /* remove timer */
}



static bool OnUseItem(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer)
{
    Au_PlayZzFX(&gSwingSound);
    struct GameLayer2DData* pData = pLayer->userData;

    HGeneric hCtx = NULL_HANDLE;
    gAxeHitContextPool = GetObjectPoolIndex(gAxeHitContextPool, &hCtx);
    
    struct AxeHitHandlerContext ctx = {
        .hEntPlayer = pPlayer->thisEntity,
        .pData = pData,
        .pLayer = pLayer
    };
    gAxeHitContextPool[hCtx] = ctx;
    struct WfPlayerEntData* pEntData = WfGetPlayerEntData(pPlayer);
    char* anim_name = pEntData->animationSet.layers[WfToolAnimationLayer].slashAnimations[pEntData->directionFacing];
    HTimer t = WfScheduleCallbackOnAnimation(pPlayer, pLayer, &ProcessAxeUsage, 0.3, anim_name, (void*)hCtx);
    return true;
}

static bool TryEquip(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer, enum WfEquipSlot slot)
{
    return false;
}


static struct WfItemDef gDef = 
{
    .UISpriteName = "basic-axe",
    .pUserData = NULL,
    .onMakeCurrent = &OnMakeCurrentItem,
    .onStopBeingCurrent = &OnStopBeingCurrentItem,
    .onUseItem = &OnUseItem,
    .onTryEquip = &TryEquip,
    .onUseAnimation = WfSlashAnim,
    .bCanUseItem = true,
    .pickupSpriteName = "basic-axe",
};

void WfAddBasicAxeDef()
{
    gAxeHitContextPool = NEW_OBJECT_POOL(struct AxeHitHandlerContext, 4);
    WfAddItemDef(&gDef);
}
