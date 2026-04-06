#include "WfWoodItem.h"
#include "WfItem.h"
#include <stdlib.h>


void WfWoodItemOnMakeCurrentItem(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer)
{

}

void WfWoodItemOnStopBeingCurrentItem(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer)
{

}

bool WfWoodItemOnUseItem(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer)
{
    return true;
}

bool WfWoodItemTryEquip(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer, enum WfEquipSlot slot)
{
    return false;
}

static struct WfItemDef gDef = 
{
    .itemName = "wood",
    .UISpriteName = "wood",
    .pUserData = NULL,
    .onMakeCurrent = &WfWoodItemOnMakeCurrentItem,
    .onStopBeingCurrent = &WfWoodItemOnStopBeingCurrentItem,
    .onUseItem = &WfWoodItemOnUseItem,
    .onTryEquip = &WfWoodItemTryEquip,
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
