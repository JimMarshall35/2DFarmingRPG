#ifndef WFWOODITEM_H
#define WFWOODITEM_H
#include <stdbool.h>
#include "WfItem.h"
#include "SharedLib.h"

struct Entity2D;
struct GameFrameworkLayer;

void WfAddWoodItemDef();

void STARDEW_API WfWoodItemOnMakeCurrentItem(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer);
void STARDEW_API WfWoodItemOnStopBeingCurrentItem(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer);
bool STARDEW_API WfWoodItemOnUseItem(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer);
bool STARDEW_API WfWoodItemTryEquip(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer, enum WfEquipSlot slot);


#endif
