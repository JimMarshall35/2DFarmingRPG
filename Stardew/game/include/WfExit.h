#ifndef WFEXIT_H
#define WFEXIT_H

#include <cglm/cglm.h>

struct BinarySerializer;
struct Entity2D;
struct GameLayer2DData;

void WfInitExit();

void WfDeSerializeExitEntity(struct BinarySerializer* bs, struct Entity2D* pOutEnt, struct GameLayer2DData* pData);

void WfSerializeExitEntity(struct BinarySerializer* bs, struct Entity2D* pInEnt, struct GameLayer2DData* pData);

/// @brief Get the players position as they left the previous area
/// @param outPos 
void WfGetPreviousAreaPosition(vec2 outPos);

#endif