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
    .pickupSpriteName = "wood"
};

void WfAddWoodItemDef()
{
    WfAddItemDef(&gDef);
}
