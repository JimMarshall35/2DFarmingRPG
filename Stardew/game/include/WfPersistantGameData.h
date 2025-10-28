#ifndef WFPERSISTANTGAMEDATA_H
#define WFPERSISTANTGAMEDATA_H

#define VECTOR(a) a*

struct WfInventoryItem
{
    int itemIndex;
    int quantity;
};

struct WfInventory
{
    VECTOR(struct WfInventoryItem) pItems;
};

void WfLoadPersistantDataFile(const char* path);

void WfSavePersistantDataFile(const char* path);

void WfNewSavePersistantData();

struct WfInventory* WfGetInventory();

#endif