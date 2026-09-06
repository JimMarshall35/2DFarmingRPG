#ifndef WFEXIT_H
#define WFEXIT_H

#include <cglm/cglm.h>
#include "HandleDefs.h"

struct BinarySerializer;
struct Entity2D;
struct GameLayer2DData;
struct Entity2DCollection;

void WfInitExit();

void WfDeSerializeExitEntity(struct BinarySerializer* bs, struct Entity2D* pOutEnt, struct GameLayer2DData* pData);

void WfSerializeExitEntity(struct BinarySerializer* bs, struct Entity2D* pInEnt, struct GameLayer2DData* pData);

/// @brief Get the players position as they left the previous area
/// @param outPos 
void WfGetPreviousAreaPosition(vec2 outPos);

HEntity2D WfAddExitAt(struct Entity2DCollection* pEntities, float x, float y, float w, float h, const char* toArea);

#endif