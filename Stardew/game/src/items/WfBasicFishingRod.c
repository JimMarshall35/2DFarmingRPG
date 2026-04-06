#include "WfBasicFishingRod.h"
#include "WfItem.h"
#include <stdlib.h>


void WfBasicFishingRodOnMakeCurrentItem(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer)
{

}

void WfBasicFishingRodOnStopBeingCurrentItem(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer)
{

}

bool WfBasicFishingRodOnUseItem(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer)
{
    return true;
}

bool WfBasicFishingRodTryEquip(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer, enum WfEquipSlot slot)
{
    return false;
}


static struct WfItemDef gDef = 
{
    .itemName = "basic-fishing-rod",
    .UISpriteName = "basic-fishing-rod",
    .pUserData = NULL,
    .onMakeCurrent = &WfBasicFishingRodOnMakeCurrentItem,
    .onStopBeingCurrent = &WfBasicFishingRodOnStopBeingCurrentItem,
    .onUseItem = &WfBasicFishingRodOnUseItem,
    .onTryEquip = &WfBasicFishingRodTryEquip,
    .pickupSpriteName = "basic-fishing-rod",
};

void WfAddBasicFishingRodDef()
{
    WfAddItemDef(&gDef);
}
