#include "WfWoodItem.h"
#include "WfBaseItemImpls.h"
#include "WfItem.h"
#include <stdlib.h>

static struct WfItemDef gDef = 
{
    .itemName = "wood",
    .UISpriteName = "wood",
    .pUserData = NULL,
    .onMakeCurrent = &WfBaseOnMakeCurrentItem,
    .onStopBeingCurrent = &WfBaseOnStopBeingCurrentItem,
    .onUseItem = &WfBaseOnUseItem,
    .onTryEquip = &WfBaseTryEquip,
    .onUseAnimation = WfNoActionAnim,
    .bCanUseItem = true,
    .pickupSpriteName = "wood",
    .bSoundEffectOnPickup = true,
    .zzfxPickup = {1.0,0.05,175.182,0.053,0.191,0.308,1,1.0,-14.38646272,167.4035968,-80.836,0.134,0.095,0.0,0.0,0.0,0.0,0.504,0.176,0.058,0.0}
};

void WfAddWoodItemDef()
{
    WfAddItemDef(&gDef);
}
