#include "WfBasicPickaxe.h"
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

    pEntData->animationSet.layers[WfToolAnimationLayer].walkAnimations[Up] = "walk-pickaxe-male-up";
    pEntData->animationSet.layers[WfToolAnimationLayer].walkAnimations[Down] = "walk-pickaxe-male-down";
    pEntData->animationSet.layers[WfToolAnimationLayer].walkAnimations[Left] = "walk-pickaxe-male-left";
    pEntData->animationSet.layers[WfToolAnimationLayer].walkAnimations[Right] = "walk-pickaxe-male-right";


    pEntData->animationSet.layers[WfToolAnimationLayer].slashAnimations[Up] = "pickaxe-slash-male-up-fg";
    pEntData->animationSet.layers[WfToolAnimationLayer].slashAnimations[Down] = "pickaxe-slash-male-down-fg";
    pEntData->animationSet.layers[WfToolAnimationLayer].slashAnimations[Left] = "pickaxe-slash-male-left-fg";
    pEntData->animationSet.layers[WfToolAnimationLayer].slashAnimations[Right] = "pickaxe-slash-male-right-fg";

    ZeroMemory(pEntData->animationSet.bgLayers[WfBG1].walkAnimations, sizeof(const char*) * NUM_ANIMATIONS);
    pEntData->animationSet.bgLayers[WfBG1].slashAnimations[Up] = "pickaxe-slash-male-up-bg";
    pEntData->animationSet.bgLayers[WfBG1].slashAnimations[Down] = "pickaxe-slash-male-down-bg";
    pEntData->animationSet.bgLayers[WfBG1].slashAnimations[Left] = "pickaxe-slash-male-left-bg";
    pEntData->animationSet.bgLayers[WfBG1].slashAnimations[Right] = "pickaxe-slash-male-right-bg";


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
    .UISpriteName = "basic-pickaxe",
    .pUserData = NULL,
    .onMakeCurrent = &OnMakeCurrentItem,
    .onStopBeingCurrent = &OnStopBeingCurrentItem,
    .onUseItem = &OnUseItem,
    .onTryEquip = &TryEquip,
    .onUseAnimation = WfSlashAnim,
    .bCanUseItem = true
};

void WfAddBasicPickaxeDef()
{
    WfAddItemDef(&gDef);
}
