#include "WfBasicPickaxe.h"
#include "WfItem.h"
#include "WfPlayer.h"
#include "Entities.h"

#include <stdlib.h>
#include "EngineUtils.h"
#include "Audio.h"
#include "WfItemHelpers.h"
#include "Maths.h"

#define PICKAXE_FAN_LENGTH (64.0F)
#define PICKAXE_FAN_WIDTH DEGREES_TO_RADIANS(45.0f)

#define PICKAXE_DAMAGE 10.0f

static struct ZZFXSound gSwingSound = {1.0,0.05,32.268,0.008,0.046,0.208,0,1.0,2.662,2.312,0.0,0.0,0.178,1.778,0.0,0.106,0.0,0.431,0.174,0.076,0.0};


void WfBasicPickaxeOnMakeCurrentItem(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer)
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

void WfBasicPickaxeOnStopBeingCurrentItem(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer)
{
    struct WfPlayerEntData* pEntData = WfGetPlayerEntData(pPlayer);
    pEntData->animationSet.layersMask &= ~(1 << WfToolAnimationLayer);
    struct Component2D* pComp = WfGetPlayerAnimationLayerComponent(pPlayer, WfToolAnimationLayer);
    pComp->data.spriteAnimator.bDraw = false;
}

bool WfBasicPickaxeTryEquip(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer, enum WfEquipSlot slot)
{
    return false;
}


static struct WfItemDef gDef = 
{
    .UISpriteName = "basic-pickaxe",
    .pUserData = NULL,
    .onMakeCurrent = &WfBasicPickaxeOnMakeCurrentItem,
    .onStopBeingCurrent = &WfBasicPickaxeOnStopBeingCurrentItem,
    .onUseItem = &WfBasicPickaxeOnUseItem,
    .onTryEquip = &WfBasicPickaxeTryEquip,
    .onUseAnimation = WfSlashAnim,
    .bCanUseItem = true,
    .pickupSpriteName = "basic-pickaxe",
};

bool WfBasicPickaxeOnUseItem(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer)
{
    Au_PlayZzFX(&gSwingSound);
    struct WfDamagingWeaponDef def = {
        .animation = WfSlashAnim,
        .damageCalculationType = DCT_Constant,
        .damageCalcData.damageConst = 10.0f,
        .damageType = WfPickaxeDamage,
        .fanLength = PICKAXE_FAN_LENGTH,
        .fanWidth = PICKAXE_FAN_WIDTH,
        .pItemDef = &gDef
    };
    return WfOnUseDamagingWeaponItem(pPlayer, pLayer, &def);
}

void WfAddBasicPickaxeDef()
{
    WfAddItemDef(&gDef);
}
