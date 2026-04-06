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

static float gAxeFanWidth = 0;
static float gAxeFanLength = 0;
static float gAxeDamage = 0;

static struct ZZFXSound gSwingSound = {1.0,0.05,32.268,0.008,0.046,0.208,0,1.0,2.662,2.312,0.0,0.0,0.178,1.778,0.0,0.106,0.0,0.431,0.174,0.076,0.0};

void WfBasicAxeOnMakeCurrentItem(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer)
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

void WfBasicAxeOnStopBeingCurrentItem(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer)
{
    struct WfPlayerEntData* pEntData = WfGetPlayerEntData(pPlayer);
    pEntData->animationSet.layersMask &= ~(1 << WfToolAnimationLayer);
    struct Component2D* pComp = WfGetPlayerAnimationLayerComponent(pPlayer, WfToolAnimationLayer);
    pComp->data.spriteAnimator.bDraw = false;
}


bool WfBasicAxeTryEquip(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer, enum WfEquipSlot slot)
{
    return false;
}

bool WfBasicAxeOnUseItem(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer);

void WfBasicAxeOnGameLayerPush(struct WfItemDef* pDef, struct GameFrameworkLayer* pLayer, DrawContext* drawContext, InputContext* inputContext)
{
    gAxeDamage = WfGetItemConfigProperty(&pDef->config, "AXE_DAMAGE")->val.floatVal;
    gAxeFanLength = WfGetItemConfigProperty(&pDef->config, "AXE_FAN_LENGTH")->val.floatVal;
    gAxeFanWidth = WfGetItemConfigProperty(&pDef->config, "AXE_FAN_WIDTH")->val.floatVal;
}

static struct WfItemDef gDef = 
{
    .itemName = "basic-axe",
    .UISpriteName = "basic-axe",
    .pUserData = NULL,
    .onMakeCurrent = &WfBasicAxeOnMakeCurrentItem,
    .onStopBeingCurrent = &WfBasicAxeOnStopBeingCurrentItem,
    .onUseItem = &WfBasicAxeOnUseItem,
    .onTryEquip = &WfBasicAxeTryEquip,
    .onUseAnimation = WfSlashAnim,
    .bCanUseItem = true,
    .pickupSpriteName = "basic-axe",
    .onGameLayerPush = &WfBasicAxeOnGameLayerPush
};

bool WfBasicAxeOnUseItem(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer)
{
    Au_PlayZzFX(&gSwingSound);
    struct WfDamagingWeaponDef def = {
        .animation = WfSlashAnim,
        .damageCalculationType = DCT_Constant,
        .damageCalcData.damageConst = gAxeDamage,
        .damageType = WfAxeDamage,
        .fanLength = gAxeFanLength,
        .fanWidth = gAxeFanWidth,
        .pItemDef = &gDef
    };
    return WfOnUseDamagingWeaponItem(pPlayer, pLayer, &def);
}

void WfAddBasicAxeDef()
{
    WfAddItemDef(&gDef);
}
