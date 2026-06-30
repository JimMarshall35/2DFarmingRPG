#include "WfBaseItemImpls.h"


void WfBaseOnMakeCurrentItem(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer)
{
}

void WfBaseOnGameLayerPush(struct WfItemDef* pDef, struct GameFrameworkLayer* pLayer, DrawContext* drawContext, InputContext* inputContext)
{
}

void WfBaseOnGameLayerPop(struct WfItemDef* pDef, struct GameFrameworkLayer* pLayer, DrawContext* drawContext, InputContext* inputContext)
{
}

void WfBaseOnStopBeingCurrentItem(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer)
{
}

bool WfBaseOnUseItem(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer)
{
    return false;
}

bool WfBaseTryEquip(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer, enum WfEquipSlot slot, struct WfItemDef* pDef)
{
    return false;
}

bool WfBaseTryUnEquip(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer, enum WfEquipSlot slot, struct WfItemDef* pDef)
{
    return false;
}

void WfBaseInitDef(struct WfItemDef*)
{

}