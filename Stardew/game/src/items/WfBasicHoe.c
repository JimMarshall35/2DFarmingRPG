#include "WfBasicHoe.h"
#include "WfItem.h"
#include <stdlib.h>
#include "WfItem.h"
#include "WfPlayer.h"
#include "Entities.h"
#include "EngineUtils.h"

#include <stdlib.h>
#include "Audio.h"

struct ZZFXSound gThrustSnd = {1.0,0.05,64.799,0.023,0.129,0.395,0,1.0,0.0,-6.535,0.0,0.0,0.161,1.945,16.886331,0.464,0.229,0.461,0.156,0.0,-3441.073};

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
    .bCanUseItem = true
};

void WfAddBasicHoeDef()
{
    WfAddItemDef(&gDef);
}
