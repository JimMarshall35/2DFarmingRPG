#include "WfEquipableItem.h"
#include <stdbool.h>
#include <string.h>
#include "StringKeyHashMap.h"
#include "WfPlayer.h"
#include "Log.h"
#include "AssertLib.h"

struct HashMap gEquipableItems = {
    .capacity = 0,
    .size = 0,
    .valueSize = 0,
    .pData = NULL
};

struct WfEquipableItem
{
    const char* name;
    struct WfAnimationSet animationSet;
    enum WfEquipSlot slot;
};

void WfEquipableMakeCurrentItem(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer)
{
}

void WfEquipableStopBeingCurrentItem(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer)
{
}

bool WfEquipableOnUseItem(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer)
{
    return true;
}

bool WfEquipableTryEquip(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer, enum WfEquipSlot slot, struct WfItemDef* pDef)
{
    struct WfEquipableItem* pEquipable = HashmapSearch(&gEquipableItems, WfGetItemConfigProperty(&pDef->config, "ITEM_NAME")->val.stringVal);
    switch (pEquipable->slot)
    {
    case Head:
        // TODO: implement
        break;
    case Torso:
        WfPlayerSetTorsoAnimationSet(pPlayer, &pEquipable->animationSet);
        break;
    case Legs:
        WfPlayerSetLegsAnimationSet(pPlayer, &pEquipable->animationSet);
        break;
    case Arms:
        // TODO: implement
        break;
    default:
        break;
    }
    return false;
}

bool WfEquipableTryUnEquip(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer, enum WfEquipSlot slot, struct WfItemDef* pDef)
{
    return true;
}

static void PopulateAnimationSetLayerFromName(struct WfAnimationSetLayer* pLayer, const char* name)
{

    ///////////////////////////////////////// walk

    char buf[256];
    sprintf(buf, "walk-%i-up", name);
    pLayer->walkAnimations[Up] = malloc(strlen(buf) + 1);
    strcpy(pLayer->walkAnimations[Up], buf);

    sprintf(buf, "walk-%i-down", name);
    pLayer->walkAnimations[Down] = malloc(strlen(buf) + 1);
    strcpy(pLayer->walkAnimations[Down], buf);

    sprintf(buf, "walk-%i-left", name);
    pLayer->walkAnimations[Left] = malloc(strlen(buf) + 1);
    strcpy(pLayer->walkAnimations[Left], buf);

    sprintf(buf, "walk-%i-right", name);
    pLayer->walkAnimations[Right] = malloc(strlen(buf) + 1);
    strcpy(pLayer->walkAnimations[Right], buf);

    ///////////////////////////////////////// slash

    sprintf(buf, "slash-%i-up", name);
    pLayer->slashAnimations[Up] = malloc(strlen(buf) + 1);
    strcpy(pLayer->slashAnimations[Up], buf);

    sprintf(buf, "slash-%i-down", name);
    pLayer->slashAnimations[Down] = malloc(strlen(buf) + 1);
    strcpy(pLayer->slashAnimations[Down], buf);

    sprintf(buf, "slash-%i-left", name);
    pLayer->slashAnimations[Left] = malloc(strlen(buf) + 1);
    strcpy(pLayer->slashAnimations[Left], buf);

    sprintf(buf, "slash-%i-right", name);
    pLayer->slashAnimations[Right] = malloc(strlen(buf) + 1);
    strcpy(pLayer->slashAnimations[Right], buf);

    ///////////////////////////////////////// thrust

    sprintf(buf, "thrust-%i-up", name);
    pLayer->thrustAnimations[Up] = malloc(strlen(buf) + 1);
    strcpy(pLayer->thrustAnimations[Up], buf);

    sprintf(buf, "thrust-%i-down", name);
    pLayer->thrustAnimations[Down] = malloc(strlen(buf) + 1);
    strcpy(pLayer->thrustAnimations[Down], buf);

    sprintf(buf, "thrust-%i-left", name);
    pLayer->thrustAnimations[Left] = malloc(strlen(buf) + 1);
    strcpy(pLayer->thrustAnimations[Left], buf);

    sprintf(buf, "thrust-%i-right", name);
    pLayer->thrustAnimations[Right] = malloc(strlen(buf) + 1);
    strcpy(pLayer->thrustAnimations[Right], buf);

}

static struct WfAnimationSet GetAnimationSetForDef(struct WfItemDef* pDef)
{
    struct WfAnimationSet r = {};
    struct WfAnimationSetLayer* pLayer = NULL;
    if (strcmp(WfGetItemConfigProperty(&pDef->config, "EQUIP_SLOT")->val.stringVal, "legs") == 0)
    {
        pLayer = &r.layers[WfLegAnimationLayer];
        PopulateAnimationSetLayerFromName(pLayer, 
            WfGetItemConfigProperty(&pDef->config, "ITEM_NAME")->val.stringVal);
    }
    else if (strcmp(WfGetItemConfigProperty(&pDef->config, "EQUIP_SLOT")->val.stringVal, "torso") == 0)
    {
        pLayer = &r.layers[WfTorsoAnimationLayer];
        PopulateAnimationSetLayerFromName(pLayer, 
            WfGetItemConfigProperty(&pDef->config, "ITEM_NAME")->val.stringVal);
    }
    else
    {
        Log_Error("Bad equip slot value %s for item %s", 
            WfGetItemConfigProperty(&pDef->config, "EQUIP_SLOT")->val.stringVal, 
            pDef->itemName);
        return r;
    }
    EASSERT(pLayer);
    PopulateAnimationSetLayerFromName(pLayer, WfGetItemConfigProperty(&pDef->config, "ITEM_NAME")->val.stringVal);
    return r;
}

static enum WfEquipSlot GetEquipSlot(const char* equipSlotName)
{
    if (strcmp(equipSlotName, "legs") == 0)
    {
        return Legs;
    }
    else if (strcmp(equipSlotName, "torso") == 0)
    {
        return Torso;
    }
    else
    {
        Log_Error("Bad equip slot value %s", equipSlotName);
        return UNKNOWN_EQUIP_SLOT;
    }
}

bool WfInitEquipable(struct WfItemDef* pDef)
{
    if(gEquipableItems.pData == NULL)
    {
        HashmapInit(&gEquipableItems, 128, sizeof(struct WfEquipableItem));
    }
    struct WfEquipableItem equipableItem = {
        .name = WfGetItemConfigProperty(&pDef->config, "ITEM_NAME")->val.stringVal,
        .animationSet = GetAnimationSetForDef(pDef),
        .slot = GetEquipSlot(WfGetItemConfigProperty(&pDef->config, "EQUIP_SLOT")->val.stringVal)
    };
    HashmapInsert(&gEquipableItems, WfGetItemConfigProperty(&pDef->config, "ITEM_NAME")->val.stringVal, &equipableItem);
}

