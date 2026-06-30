#ifndef WFEQUIPABLEITEM_H
#define WFEQUIPABLEITEM_H
#include <stdbool.h>
#include "WfItem.h"
#include "SharedLib.h"

struct GameFrameworkLayer;
struct Entity2D;

void STARDEW_API WfEquipableMakeCurrentItem(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer);

void STARDEW_API WfEquipableStopBeingCurrentItem(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer);

bool STARDEW_API WfEquipableOnUseItem(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer);

bool STARDEW_API WfEquipableTryEquip(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer, enum WfEquipSlot slot, struct WfItemDef* pDef);

bool STARDEW_API WfEquipableTryUnEquip(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer, enum WfEquipSlot slot, struct WfItemDef* pDef);

bool STARDEW_API WfInitEquipable(struct WfItemDef* pDef);

#endif