#include "WfBasicBow.h"
#include "WfItem.h"
#include <stdlib.h>

void WfBasicBowOnMakeCurrentItem(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer)
{

}

void WfBasicBowOnStopBeingCurrentItem(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer)
{

}

bool WfBasicBowOnUseItem(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer)
{
    return true;
}

bool WfBasicBowTryEquip(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer, enum WfEquipSlot slot)
{
    return false;
}


static struct WfItemDef gDef = 
{
    .UISpriteName = "basic-bow",
    .pUserData = NULL,
    .onMakeCurrent = &WfBasicBowOnMakeCurrentItem,
    .onStopBeingCurrent = &WfBasicBowOnStopBeingCurrentItem,
    .onUseItem = &WfBasicBowOnUseItem,
    .onTryEquip = &WfBasicBowTryEquip,
    .pickupSpriteName = "basic-bow",
};

void WfAddBasicBowDef()
{
    WfAddItemDef(&gDef);
}
