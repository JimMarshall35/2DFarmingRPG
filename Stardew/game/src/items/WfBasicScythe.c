#include "WfBasicScythe.h"
#include "WfItem.h"
#include <stdlib.h>


void WfBasicScytheOnMakeCurrentItem(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer)
{

}

void WfBasicScytheOnStopBeingCurrentItem(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer)
{

}

bool WfBasicScytheOnUseItem(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer)
{
    return true;
}

bool WfBasicScytheTryEquip(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer, enum WfEquipSlot slot)
{
    return false;
}


static struct WfItemDef gDef = 
{
    .UISpriteName = "basic-scythe",
    .pUserData = NULL,
    .onMakeCurrent = &WfBasicScytheOnMakeCurrentItem,
    .onStopBeingCurrent = &WfBasicScytheOnStopBeingCurrentItem,
    .onUseItem = &WfBasicScytheOnUseItem,
    .onTryEquip = &WfBasicScytheTryEquip,
    .pickupSpriteName = "basic-scythe",
};

void WfAddBasicScytheDef()
{
    WfAddItemDef(&gDef);
}

