#ifndef WFBASICHOE_H
#define WFBASICHOE_H
#include "DrawContext.h"
#include "InputContext.h"
#include "WfItem.h"
#include "SharedLib.h"

struct Entity2D;
struct GameFrameworkLayer;


void STARDEW_API WfBasicHoeOnMakeCurrentItem(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer);
void STARDEW_API WfBasicHoeOnGameLayerPush(struct WfItemDef* pDef, struct GameFrameworkLayer* pLayer, DrawContext* drawContext, InputContext* inputContext);
void STARDEW_API WfBasicHoeOnGameLayerPop(struct WfItemDef* pDef, struct GameFrameworkLayer* pLayer, DrawContext* drawContext, InputContext* inputContext);
void STARDEW_API WfBasicHoeOnStopBeingCurrentItem(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer);
bool STARDEW_API WfBasicHoeOnUseItem(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer);



#endif
