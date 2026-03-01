#ifndef WFITEMPICKUP_H
#define WFITEMPICKUP_H
#include "HandleDefs.h"

struct BinarySerializer;
struct Entity2D;
struct GameLayer2DData;

struct WfItemPickupDef
{
    int itemID;
    int itemQuantity;
};

void WfDeSerializeItemPickupEntity(struct BinarySerializer* bs, struct Entity2D* pOutEnt, struct GameLayer2DData* pData);

void WfSerializeItemPickupEntity(struct BinarySerializer* bs, struct Entity2D* pInEnt, struct GameLayer2DData* pData);

HEntity2D WfAddPickupBasedAt(float x, float y, struct WfItemPickupDef* def, struct GameLayer2DData* pGameLayerData);

void WfInitItemPickup();

#endif