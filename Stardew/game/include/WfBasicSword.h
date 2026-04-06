#ifndef WFBASICSWORD_H
#define WFBASICSWORD_H
#include <stdbool.h>
#include "SharedLib.h"
#include "WfItem.h"

struct GameFrameworkLayer;
struct Entity2D;

void STARDEW_API WfAddBasicSwordDef();
bool STARDEW_API WfBasicSwordTryEquip(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer, enum WfEquipSlot slot);
bool STARDEW_API WfBasicSwordOnUseItem(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer);
void STARDEW_API WfBasicSwordOnStopBeingCurrentItem(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer);
void STARDEW_API WfBasicSwordOnMakeCurrentItem(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer);

#endif
