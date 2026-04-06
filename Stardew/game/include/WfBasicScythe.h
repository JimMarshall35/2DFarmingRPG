#ifndef WFBASICSCYTHE_H
#define WFBASICSCYTHE_H
#include <stdbool.h>
#include "SharedLib.h"
#include "WfItem.h"

struct GameFrameworkLayer;
struct Entity2D;

void STARDEW_API WfAddBasicScytheDef();
void STARDEW_API WfBasicScytheOnMakeCurrentItem(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer);
void STARDEW_API WfBasicScytheOnStopBeingCurrentItem(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer);
bool STARDEW_API WfBasicScytheOnUseItem(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer);
bool STARDEW_API WfBasicScytheTryEquip(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer, enum WfEquipSlot slot);

#endif
