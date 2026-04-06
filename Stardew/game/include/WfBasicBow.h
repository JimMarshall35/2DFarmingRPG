#ifndef WFBASICBOW_H
#define WFBASICBOW_H
#include <stdbool.h>
#include "WfItem.h"
#include "SharedLib.h"

struct Entity2D;
struct GameFrameworkLayer;

void WfAddBasicBowDef();

void STARDEW_API WfBasicBowOnMakeCurrentItem(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer);
void STARDEW_API WfBasicBowOnStopBeingCurrentItem(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer);
bool STARDEW_API WfBasicBowOnUseItem(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer);
bool STARDEW_API WfBasicBowTryEquip(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer, enum WfEquipSlot slot);



#endif