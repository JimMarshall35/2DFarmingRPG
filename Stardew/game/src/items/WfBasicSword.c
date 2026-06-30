#include "WfBasicSword.h"
#include "WfItem.h"
#include "WfPlayer.h"
#include "Entities.h"

#include <stdlib.h>

void WfBasicSwordOnMakeCurrentItem(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer)
{
    // struct WfPlayerEntData* pEntData = WfGetPlayerEntData(pPlayer);
    // pEntData->animationSet.layersMask |= (1 << WfToolAnimationLayer);
    // pEntData->animationSet.layers[WfToolAnimationLayer].animationNames[Up] = "walk-longsword-male-up";
    // pEntData->animationSet.layers[WfToolAnimationLayer].animationNames[Down] = "walk-longsword-male-down";
    // pEntData->animationSet.layers[WfToolAnimationLayer].animationNames[Left] = "walk-longsword-male-left";
    // pEntData->animationSet.layers[WfToolAnimationLayer].animationNames[Right] = "walk-longsword-male-right";

    // struct Component2D* pComp = WfGetPlayerAnimationLayerComponent(pPlayer, WfToolAnimationLayer);
    // WfSetPlayerOverlayAnimations(pEntData->directionFacing, pLayer, pEntData, pPlayer);
    // pComp->data.spriteAnimator.onSprite = 0;
}

void WfBasicSwordOnStopBeingCurrentItem(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer)
{
    // struct WfPlayerEntData* pEntData = WfGetPlayerEntData(pPlayer);
    // pEntData->animationSet.layersMask &= ~(1 << WfToolAnimationLayer);
    // struct Component2D* pComp = WfGetPlayerAnimationLayerComponent(pPlayer, WfToolAnimationLayer);
    // pComp->data.spriteAnimator.bDraw = false;
}

bool WfBasicSwordOnUseItem(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer)
{
    return true;
}
