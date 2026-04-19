#ifndef WFROCK_H
#define WFROCK_H

#include <cglm/cglm.h>
#include "HandleDefs.h"

struct BinarySerializer;
struct Entity2D;
struct GameLayer2DData;

enum WfDebrisType
{
    WfRockType1,
    WfRockType2,
    WfLogType1,
};

struct WfDebrisDef
{
    enum WfDebrisType type;
    float health;
};

void WfDeSerializeDebrisEntity(struct BinarySerializer* bs, struct Entity2D* pOutEnt, struct GameLayer2DData* pData);

void WfSerializeDebrisEntity(struct BinarySerializer* bs, struct Entity2D* pInEnt, struct GameLayer2DData* pData);

void WfMakeEntityIntoDebrisBasedAt(struct Entity2D* pEnt, int xTile, int yTile, struct WfDebrisDef* def, struct GameLayer2DData* pGameLayerData);

void WfDebrisGetGroundContactPoint(struct Entity2D* pEnt, vec2 outPoint);

HEntity2D WfAddDebrisBasedAt(float x, float y, struct WfDebrisDef* def, struct GameLayer2DData* pGameLayerData);

#endif