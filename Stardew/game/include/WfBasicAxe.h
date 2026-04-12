#ifndef WFBASICAXE_H
#define WFBASICAXE_H
#include "SharedLib.h"
#include <stdbool.h>
#include "WfItem.h"

struct Entity2D;
struct GameFrameworkLayer;

bool STARDEW_API WfBasicAxeOnUseItem(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer);
void STARDEW_API WfBasicAxeOnMakeCurrentItem(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer);
void STARDEW_API WfBasicAxeOnStopBeingCurrentItem(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer);
bool STARDEW_API WfBasicAxeTryEquip(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer, enum WfEquipSlot slot);
void STARDEW_API WfBasicAxeOnGameLayerPush(struct WfItemDef* pDef, struct GameFrameworkLayer* pLayer, DrawContext* drawContext, InputContext* inputContext);

#endif