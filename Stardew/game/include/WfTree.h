#ifndef WFTREE_H
#define WFTREE_H

#include "HandleDefs.h"
#include "WfEnums.h"
#include <cglm/cglm.h>

struct BinarySerializer;
struct Entity2D;
struct GameLayer2DData;
struct Entity2DCollection;


enum WfTreeType
{
    Coniferous,
    Deciduous
};

struct WfTreeDef
{
    enum WfSeason season;
    enum WfTreeType type;
    int subtype;
};

void WfDeSerializeTreeEntity(struct BinarySerializer* bs, struct Entity2D* pOutEnt, struct GameLayer2DData* pData);

void WfSerializeTreeEntity(struct BinarySerializer* bs, struct Entity2D* pInEnt, struct GameLayer2DData* pData);

HEntity2D WfAddTreeBasedAt(float x, float y, struct WfTreeDef* def, struct GameLayer2DData* pGameLayerData);

void WfTreeInit();

void WfTreeGetGroundContactPoint(struct Entity2D* pTreeEnt, vec2 outPos);

#endif