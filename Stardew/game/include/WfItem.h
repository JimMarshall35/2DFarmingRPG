#ifndef WFITEM_H
#define WFITEM_H
#include "HandleDefs.h"
#include <stdbool.h>

#define UI_SPRITE_DIMS_PXLS 32
struct Entity2D;
struct GameFrameworkLayer;

enum WfEquipSlot
{
    Ring1,
    Ring2,
    Head,
    Torso,
    Legs,
    Arms
};

/* when it is switched to in the menu */
typedef void(*OnMakeItemCurrentFn)(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer);
typedef void(*OnStopBeingCurrentItemFn)(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer);
typedef bool(*OnUseItemFn)(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer); // return false if item is used up
typedef bool(*TryEquipFn)(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer, enum WfEquipSlot slot);

struct WfItemDef
{
    /* sprite shown in UI menus */
    const char* UISpriteName;
    void* pUserData;
    OnMakeItemCurrentFn onMakeCurrent;
    OnStopBeingCurrentItemFn onStopBeingCurrent;
    OnUseItemFn onUseItem;
    TryEquipFn onTryEquip;
};

void WfAddItemDef(struct WfItemDef* pDef);

void WfAddBuiltinItems();

void WfInitItems();

void WfRegisterItemScriptFunctions();

#endif
