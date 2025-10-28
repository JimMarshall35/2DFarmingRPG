#include "WfPersistantGameData.h"
#include "DynArray.h"
#include "WfItem.h"
#include "BinarySerializer.h"
#include "AssertLib.h"

#define WF_INVENTORY_SIZE_INITIAL 12


struct WfPersistantData
{
    struct WfInventory inventory;
};

struct WfPersistantData gPersistantData;

void WfPersistantDataInit()
{
    gPersistantData.inventory.pItems = NEW_VECTOR(struct WfInventoryItem);
}


void WfLoadPersistantDataFileV1(struct BinarySerializer* pBS)
{
    u32 size = 0;
    BS_DeSerializeU32(&size, pBS);
    gPersistantData.inventory.pItems = VectorResize(gPersistantData.inventory.pItems, size);
    gPersistantData.inventory.pItems = VectorClear(gPersistantData.inventory.pItems);
    for(int i = 0; i < size; i++)
    {
        struct WfInventoryItem item;
        BS_DeSerializeI32(&item.itemIndex, pBS);
        BS_DeSerializeI32(&item.quantity, pBS);
        gPersistantData.inventory.pItems = VectorPush(gPersistantData.inventory.pItems, &item);
    }
}

void WfLoadPersistantDataFile(const char* path)
{
	struct BinarySerializer bs;
    BS_CreateForLoad(path, &bs);
    u32 version = 0;
    BS_DeSerializeU32(&version, &bs);
    switch (version)
    {
    case 1:
        WfLoadPersistantDataFileV1(&bs);
        break;
    default:
        EASSERT(false);
        break;
    }

    BS_Finish(&bs);
}

void WfSavePersistantDataFile(const char* path)
{
    struct BinarySerializer bs;
    BS_CreateForSave(path, &bs);

    BS_SerializeU32(1, &bs);

    int sz = VectorSize(gPersistantData.inventory.pItems);
    BS_SerializeU32(sz, &bs);

    for(int i=0; i<VectorSize(gPersistantData.inventory.pItems); i++)
    {
        BS_SerializeI32(gPersistantData.inventory.pItems[i].itemIndex, &bs);
        BS_SerializeI32(gPersistantData.inventory.pItems[i].quantity, &bs);
    }

    BS_Finish(&bs);
}

void WfNewSavePersistantData()
{
    gPersistantData.inventory.pItems = VectorResize(gPersistantData.inventory.pItems, WF_INVENTORY_SIZE_INITIAL);
    gPersistantData.inventory.pItems = VectorClear(gPersistantData.inventory.pItems);
    for(int i=0; i<WF_INVENTORY_SIZE_INITIAL; i++)
    {
        struct WfInventoryItem itm;
        itm.itemIndex = -1;
        itm.quantity = 0;
        gPersistantData.inventory.pItems = VectorPush(gPersistantData.inventory.pItems, &itm);
    }
    for(int i=0; i<WfNumBuiltinItems; i++)
    {
        gPersistantData.inventory.pItems[i].itemIndex = i;
        gPersistantData.inventory.pItems[i].quantity = 1;
    }
}

struct WfInventory* WfGetInventory()
{
    return &gPersistantData.inventory;
}