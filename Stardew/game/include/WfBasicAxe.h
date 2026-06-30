#ifndef WFBASICAXE_H
#define WFBASICAXE_H
#include "SharedLib.h"
#include <stdbool.h>
#include "WfItem.h"
#include "DrawContext.h"

struct Transform2D;
struct Entity2D;
struct GameFrameworkLayer;

bool STARDEW_API WfBasicAxeOnUseItem(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer);
void STARDEW_API WfBasicAxeOnMakeCurrentItem(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer);
void STARDEW_API WfBasicAxeOnStopBeingCurrentItem(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer);
void STARDEW_API WfBasicAxeOnGameLayerPush(struct WfItemDef* pDef, struct GameFrameworkLayer* pLayer, DrawContext* drawContext, InputContext* inputContext);
void STARDEW_API WfBasicAxeDrawDebugLines(struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer, struct Transform2D* pCam, VECTOR(WorldspaceLineVertex)* outVerts, struct WfItemDef* pItemDef);
#endif