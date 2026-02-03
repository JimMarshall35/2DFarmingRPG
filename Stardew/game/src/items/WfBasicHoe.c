#include "WfBasicHoe.h"
#include "WfItem.h"
#include <stdlib.h>
#include "WfItem.h"
#include "WfPlayer.h"
#include "Entities.h"

#include <stdlib.h>


static void OnMakeCurrentItem(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer)
{
    struct WfPlayerEntData* pEntData = WfGetPlayerEntData(pPlayer);
    pEntData->animationSet.layersMask |= (1 << WfToolAnimationLayer);
    pEntData->animationSet.bgLayersMask = 0;

    pEntData->animationSet.layers[WfToolAnimationLayer].walkAnimations[Up] = "walk-hoe-male-up";
    pEntData->animationSet.layers[WfToolAnimationLayer].walkAnimations[Down] = "walk-hoe-male-down";
    pEntData->animationSet.layers[WfToolAnimationLayer].walkAnimations[Left] = "walk-hoe-male-left";
    pEntData->animationSet.layers[WfToolAnimationLayer].walkAnimations[Right] = "walk-hoe-male-right";

    pEntData->animationSet.layers[WfToolAnimationLayer].thrustAnimations[Up] = "hoe-thrust-male-up-fg";
    pEntData->animationSet.layers[WfToolAnimationLayer].thrustAnimations[Down] = "hoe-thrust-male-down-fg";
    pEntData->animationSet.layers[WfToolAnimationLayer].thrustAnimations[Left] = "hoe-thrust-male-left-fg";
    pEntData->animationSet.layers[WfToolAnimationLayer].thrustAnimations[Right] = "hoe-thrust-male-right-fg";

    pEntData->animationSet.bgLayers[WfToolAnimationLayer].thrustAnimations[Up] = "hoe-thrust-male-bg";
    pEntData->animationSet.bgLayers[WfToolAnimationLayer].thrustAnimations[Down] = "hoe-thrust-male-bg";
    pEntData->animationSet.bgLayers[WfToolAnimationLayer].thrustAnimations[Left] = "hoe-thrust-male-bg";
    pEntData->animationSet.bgLayers[WfToolAnimationLayer].thrustAnimations[Right] = "hoe-thrust-male-bg";


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
    .onTryEquip = &TryEquip
};

void WfAddBasicHoeDef()
{
    WfAddItemDef(&gDef);
}
