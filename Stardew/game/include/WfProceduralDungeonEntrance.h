#ifndef WFPROCEDURALDUNGEONENTRANCE_H
#define WFPROCEDURALDUNGEONENTRANCE_H


#include <cglm/cglm.h>
#include "HandleDefs.h"

struct BinarySerializer;
struct Entity2D;
struct GameLayer2DData;
struct Entity2DCollection;

void WfInitProceduralDungeonEntrance();

void WfDeSerializeProceduralDungeonEntranceEntity(struct BinarySerializer* bs, struct Entity2D* pOutEnt, struct GameLayer2DData* pData);

void WfSerializeProceduralDungeonEntranceEntity(struct BinarySerializer* bs, struct Entity2D* pInEnt, struct GameLayer2DData* pData);

HEntity2D WfAddProceduralDungeonEntranceAt(struct Entity2DCollection* pEntities, float x, float y, float w, float h, const char* genScript, const char* genFn);

#endif