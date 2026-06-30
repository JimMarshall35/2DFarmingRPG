#ifndef WFITEM_H
#define WFITEM_H
#include "HandleDefs.h"
#include <stdbool.h>
#include "WfEnums.h"
#include "ZzFX.h"
#include "DrawContext.h"
#include "InputContext.h"
#include "StringKeyHashMap.h"

#define UI_SPRITE_DIMS_PXLS 32
struct Entity2D;
struct GameFrameworkLayer;
struct WfItemDef;

enum WfItemIndices
{
    WfBasicAxe,
    WfBasicSword,
    WfBasicPickAxe,
    WfBasicScythe,
    WfBasicFishingRod,
    WfBasicHoe,
    WfBasicBow,
    WfWoodItem,
    WfNumBuiltinItems
};

enum WfEquipSlot
{
    Ring1,
    Ring2,
    Head,
    Torso,
    Legs,
    Arms,
    UNKNOWN_EQUIP_SLOT
};

/* when it is switched to in the menu */
typedef void(*OnMakeItemCurrentFn)(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer);
typedef void(*OnStopBeingCurrentItemFn)(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer);
typedef bool(*OnUseItemFn)(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer); // return false if item is used up
typedef bool(*TryEquipFn)(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer, enum WfEquipSlot slot, struct WfItemDef* pDef);
typedef bool(*TryUnEquipFn)(struct Entity2D* pPlayer, struct GameFrameworkLayer* pLayer, enum WfEquipSlot slot, struct WfItemDef* pDef);

/* Initialize the item definition itself */
typedef void(*OnInitializeItemDefFn)(struct WfItemDef*);

typedef void (*ItemDrawDebugLinesFn)(struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer, struct Transform2D* pCam, VECTOR(WorldspaceLineVertex)* outVerts, struct WfItemDef* pItemDef);


/// @brief called when the game layer is pushed.
// An item definition might use this to initialize data from the atlas such as sprites or named tiles
typedef void(*OnPushGameLayerItemFnItemFn)(struct WfItemDef* pDef, struct GameFrameworkLayer* pLayer, DrawContext* drawContext, InputContext* inputContext);

/// @brief called when the game layer is popped
typedef void(*OnPopGameLayerItemFnItemFn)(struct WfItemDef* pDef, struct GameFrameworkLayer* pLayer, DrawContext* drawContext, InputContext* inputContext);

enum WfItemConfigPropertyType
{
    WfItemConfig_Float,
    WfItemConfig_Int,
    WfItemConfig_Bool,
    WfItemConfig_String,
    WfItemConfig_Array,
    WfItemConfig_Bag,
};

struct WfItemConfigPropertyBag
{
    /// @brief map of keys to struct WfItemConfigProperty values
    struct HashMap properties;
    bool bSet;
};

struct WfItemConfigProperty
{
    char* name;
    enum WfItemConfigPropertyType type;
    union
    {
        float floatVal;
        int intVal;
        bool boolVal;
        char* stringVal;
        struct
        {
            int length;
            struct WfItemConfigProperty* pData;
        }propertyArray;
        struct WfItemConfigPropertyBag bag;
    }val;
    
};

struct WfItemDef
{
    /// @brief sprite shown in UI menus
    const char* UISpriteName;

    /// @brief pretty much pointless
    void* pUserData;

    /// @brief the item is selected for use
    OnMakeItemCurrentFn onMakeCurrent;

    /// @brief player switches to another item in their inventory
    OnStopBeingCurrentItemFn onStopBeingCurrent;

    /// @brief The player tries to use the item while playing
    OnUseItemFn onUseItem;

    /// @brief This isn't used yet, it is for when the player will try to equip a wearable item in their inventory menu
    TryEquipFn onTryEquip;

    TryUnEquipFn onTryUnequip;

    /// @brief called when the game layer is pushed.
    OnPushGameLayerItemFnItemFn onGameLayerPush;

    /// @brief called when the game layer is popped.
    OnPushGameLayerItemFnItemFn onGameLayerPop;

    /// @brief called by the player if not null to draw debug lines
    ItemDrawDebugLinesFn drawDebugLines;

    /// @brief initialize the item def, it might want to read config data and use it to initialize a C struct, etc
    OnInitializeItemDefFn init;

    /// @brief The animation applied to the player when the item is used
    enum WfActionAnimation onUseAnimation;

    /// @brief CAN this item be used at all?
    bool bCanUseItem;

    /// @brief the name of the sprite for the item as it appears in the world as an object that can be picked up
    const char* pickupSpriteName;

    /// @brief do we play a sound effect when we pick it up
    bool bSoundEffectOnPickup;

    /// @brief zzfx sound effect to play (will be union with a wav sound effect in future)
    struct ZZFXSound zzfxPickup;

    /// @brief name that gameplay code can lookup the item handle by
    char* itemName;

    /// @brief read only properties set through xml. Allows basic aspects of item behavior to be configured without recompiling
    struct WfItemConfigPropertyBag config;
    
};

void WfAddItemDef(struct WfItemDef* pDef);

void WfAddBuiltinItems();

void WfInitItems();

void WfRegisterItemScriptFunctions();

const struct WfItemDef* WfGetItemDef(int itemIndex);

struct WfItemDef* WfGetItemDefs(int* numDefs);

struct WfItemConfigProperty* WfGetItemConfigProperty(struct WfItemConfigPropertyBag* pBag, const char* propertyName);

#endif
