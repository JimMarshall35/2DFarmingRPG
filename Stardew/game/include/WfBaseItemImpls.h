#ifndef WFBASE_ITEM_IMPLS_H
#define WFBASE_ITEM_IMPLS_H

#include "DrawContext.h"
#include "InputContext.h"
#include "WfItem.h"
#include "SharedLib.h"

struct Entity2D;
struct GameFrameworkLayer;


void STARDEW_API WfBaseOnMakeCurrentItem(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer);
void STARDEW_API WfBaseOnGameLayerPush(struct WfItemDef* pDef, struct GameFrameworkLayer* pLayer, DrawContext* drawContext, InputContext* inputContext);
void STARDEW_API WfBaseOnGameLayerPop(struct WfItemDef* pDef, struct GameFrameworkLayer* pLayer, DrawContext* drawContext, InputContext* inputContext);
void STARDEW_API WfBaseOnStopBeingCurrentItem(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer);
bool STARDEW_API WfBaseOnUseItem(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer);
bool STARDEW_API WfBaseTryEquip(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer, enum WfEquipSlot slot,  struct WfItemDef* pDef);
bool STARDEW_API WfBaseTryUnEquip(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer, enum WfEquipSlot slot, struct WfItemDef* pDef);
void STARDEW_API WfBaseInitDef(struct WfItemDef* pDef);

#endif