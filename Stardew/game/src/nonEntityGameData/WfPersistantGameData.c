#include "WfPersistantGameData.h"
#include "DynArray.h"
#include "WfItem.h"
#include "BinarySerializer.h"
#include "AssertLib.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


static struct WfPersistantData gPersistantData;

static struct WfPersistantData gNetworkPlayersPersistantData[3];

static int gOccupiedSpots = 0;

int WfGetNumNetworkPlayerPersistentDataSlots()
{
    return gOccupiedSpots;
}

void WfSetNumNetworkPlayerPersistentDataSlots(int num)
{
    gOccupiedSpots = num;
}

struct WfPersistantData* WfGetLocalPlayerPersistantGameData()
{
    return &gPersistantData;
}

struct WfPersistantData* WfGetNetworkPlayerPersistantGameData(int playerNum)
{
    return &gNetworkPlayersPersistantData[playerNum];
}

void WfPersistantDataInit()
{
    memset(&gPersistantData, 0, sizeof(struct WfPersistantData));
    gPersistantData.inventory.pItems = NEW_VECTOR(struct WfInventoryItem);
    memset(&gNetworkPlayersPersistantData, 0, sizeof(struct WfPersistantData) * 3);
    for(int i=0; i<3; i++)
    {
        gNetworkPlayersPersistantData->inventory.pItems = NEW_VECTOR(struct WfInventoryItem);
    }
}

void WfLoadPersistantDataFileV1(struct BinarySerializer* pBS, struct WfPersistantData* pData)
{
    BS_DeSerializeI32(&pData->inventory.selectedItem, pBS);
    u32 size = 0;
    BS_DeSerializeU32(&size, pBS);
    pData->inventory.pItems = VectorResize(pData->inventory.pItems, size);
    pData->inventory.pItems = VectorClear(pData->inventory.pItems);
    for(int i = 0; i < size; i++)
    {
        struct WfInventoryItem item;
        BS_DeSerializeI32(&item.itemIndex, pBS);
        BS_DeSerializeI32(&item.quantity, pBS);
        pData->inventory.pItems = VectorPush(pData->inventory.pItems, &item);
    }

    BS_DeSerializeFloat(&pData->preferences.zoomLevel, pBS);
}

void WfLoadPersistantDataFileInternal(struct BinarySerializer* pBS, struct WfPersistantData* pData)
{
    u32 version = 0;
    BS_DeSerializeU32(&version, pBS);
    switch (version)
    {
    case 1:
        WfLoadPersistantDataFileV1(pBS, pData);
        break;
    default:
        EASSERT(false);
        break;
    }
}

void WfLoadPersistantDataFile(const char* path)
{
	struct BinarySerializer bs;
    BS_CreateForLoad(path, &bs);
    WfLoadPersistantDataFileInternal(&bs, &gPersistantData);

    BS_Finish(&bs);
}

void WfSavePersistantDataFileInternal(struct BinarySerializer* pBS, struct WfPersistantData* pGameData)
{
    BS_SerializeU32(1, pBS); /* version */

    /* Inventory */

    BS_SerializeI32(pGameData->inventory.selectedItem, pBS);

    int sz = VectorSize(pGameData->inventory.pItems);
    BS_SerializeU32(sz, pBS);

    for(int i=0; i<VectorSize(pGameData->inventory.pItems); i++)
    {
        BS_SerializeI32(pGameData->inventory.pItems[i].itemIndex, pBS);
        BS_SerializeI32(pGameData->inventory.pItems[i].quantity, pBS);
    }

    /* Preferences */
    BS_SerializeFloat(pGameData->preferences.zoomLevel, pBS);
}

void WfSavePersistantDataFile(const char* path)
{
    struct BinarySerializer bs;
    BS_CreateForSave(path, &bs);
    WfSavePersistantDataFileInternal(&bs, &gPersistantData);
    BS_Finish(&bs);
}

void WfNewSavePersistantData()
{
    /* Inventory */
    gPersistantData.inventory.pItems = VectorResize(gPersistantData.inventory.pItems, WF_INVENTORY_SIZE_INITIAL);
    gPersistantData.inventory.pItems = VectorClear(gPersistantData.inventory.pItems);
    for(int i=0; i<WF_INVENTORY_SIZE_INITIAL; i++)
    {
        struct WfInventoryItem itm;
        itm.itemIndex = -1;
        itm.quantity = 0;
        gPersistantData.inventory.pItems = VectorPush(gPersistantData.inventory.pItems, &itm);
    }

    gPersistantData.inventory.pItems[0].itemIndex = WfBasicAxe;
    gPersistantData.inventory.pItems[0].quantity = 1;

    gPersistantData.inventory.pItems[1].itemIndex = WfBasicPickAxe;
    gPersistantData.inventory.pItems[1].quantity = 1;

    gPersistantData.inventory.pItems[2].itemIndex = WfBasicHoe;
    gPersistantData.inventory.pItems[2].quantity = 1;

    gPersistantData.inventory.selectedItem = 0;

    /* Preferences */
    gPersistantData.preferences.zoomLevel = 1.0f;
    
}

struct WfInventory* WfGetInventory()
{
    return &gPersistantData.inventory;
}

struct WfInventory* WfGetNetworkPlayersInventory(int player)
{
    EASSERT(player <= 2 && player >= 0);
    return &gNetworkPlayersPersistantData[player].inventory;
}

struct WfPlayerPreferences* WfGetPreferences()
{
    return &gPersistantData.preferences;
}

/// @brief search for existing stacks of the item
/// @param pInv 
/// @param item 
/// @return the indices in the inventory of stacks or empty it none
static VECTOR(int) SearchForItemStackIndices(struct WfInventory* pInv, int item, VECTOR(int) inIndices)
{
    inIndices = VectorClear(inIndices);
    for(int i=0; i<VectorSize(pInv->pItems); i++)
    {
        if(pInv->pItems[i].itemIndex == item)
        {
            inIndices = VectorPush(inIndices, &i);
        }
    }
    return inIndices;
}

bool WfAddToInventory(struct WfInventory* pInv, int item, int quantity)
{
    static VECTOR(int) sStackIndices = NULL;
    if(sStackIndices == NULL)
    {
        sStackIndices = NEW_VECTOR(int);
        sStackIndices = VectorResize(sStackIndices, 8);
    }
    sStackIndices = SearchForItemStackIndices(pInv, item, sStackIndices);
    if(VectorSize(sStackIndices) > 0)
    {
        /* if there's an existing stack add to that */
        pInv->pItems[sStackIndices[0]].quantity += quantity;
    }
    else
    {
        /* else add to an empty space;*/
        for(int i=0; i<VectorSize(pInv->pItems); i++)
        {
            if(pInv->pItems[i].itemIndex == -1)
            {
                pInv->pItems[i].itemIndex = item;
                pInv->pItems[i].quantity = quantity;
                return true;
            }
        }
    }
    return false;
}