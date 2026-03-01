#include "WfWoodItem.h"
#include "WfItem.h"
#include <stdlib.h>

static void OnMakeCurrentItem(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer)
{

}

static void OnStopBeingCurrentItem(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer)
{

}

static bool ProcessAxeUsage(struct SDTimer* pTimer)
{
    return true; /* remove timer */
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
    .UISpriteName = "wood",
    .pUserData = NULL,
    .onMakeCurrent = &OnMakeCurrentItem,
    .onStopBeingCurrent = &OnStopBeingCurrentItem,
    .onUseItem = &OnUseItem,
    .onTryEquip = &TryEquip,
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
