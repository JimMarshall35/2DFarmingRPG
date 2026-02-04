#include "WfBasicAxe.h"
#include "WfItem.h"
#include "WfPlayer.h"
#include "Entities.h"

#include <stdlib.h>
#include "EngineUtils.h"

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
    .UISpriteName = "basic-axe",
    .pUserData = NULL,
    .onMakeCurrent = &OnMakeCurrentItem,
    .onStopBeingCurrent = &OnStopBeingCurrentItem,
    .onUseItem = &OnUseItem,
    .onTryEquip = &TryEquip,
    .onUseAnimation = WfSlashAnim,
    .bCanUseItem = true
};

void WfAddBasicAxeDef()
{
    WfAddItemDef(&gDef);
}
