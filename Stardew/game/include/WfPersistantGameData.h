#ifndef WFPERSISTANTGAMEDATA_H
#define WFPERSISTANTGAMEDATA_H

#define VECTOR(a) a*

#define WF_INVENTORY_ROW_SIZE 12
#define WF_INVENTORY_SIZE_INITIAL WF_INVENTORY_ROW_SIZE

#include <stdbool.h>

struct BinarySerializer;
struct WfPersistantData;


struct WfPlayerPreferences
{
    float zoomLevel;
};

struct WfInventoryItem
{
    int itemIndex;
    int quantity;
};

struct WfInventory
{
    VECTOR(struct WfInventoryItem) pItems;
    int selectedItem;

    int torsoItem;
    int legItem;
};

struct WfPersistantData
{
    struct WfInventory inventory;
    struct WfPlayerPreferences preferences;
};

void WfLoadPersistantDataFile(const char* path);

void WfSavePersistantDataFile(const char* path);

void WfNewSavePersistantData();

struct WfInventory* WfGetInventory();

struct WfPlayerPreferences* WfGetPreferences();

void WfSavePersistantDataFileInternal(struct BinarySerializer* bs, struct WfPersistantData* pGameData);

void WfLoadPersistantDataFileInternal(struct BinarySerializer* pBS, struct WfPersistantData* pData);

struct WfPersistantData* WfGetLocalPlayerPersistantGameData();

void WfPersistantDataInit();

bool WfAddToInventory(struct WfInventory* pInv, int item, int quantity);

#endif