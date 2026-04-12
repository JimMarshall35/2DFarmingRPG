#ifndef WFBASICPICKAXE_H
#define WFBASICPICKAXE_H
#include <stdbool.h>
#include "SharedLib.h"
#include "WfItem.h"

struct Entity2D;
struct GameFrameworkLayer;
void STARDEW_API WfBasicPickaxeOnMakeCurrentItem(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer);
void STARDEW_API WfBasicPickaxeOnStopBeingCurrentItem(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer);
bool STARDEW_API WfBasicPickaxeTryEquip(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer, enum WfEquipSlot slot);
bool STARDEW_API WfBasicPickaxeOnUseItem(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer);
void STARDEW_API WfBasicPickAxeOnGameLayerPush(struct WfItemDef* pDef, struct GameFrameworkLayer* pLayer, DrawContext* drawContext, InputContext* inputContext);

#endif
