#ifndef WFPROCEDURALDUNGEONENTRANCE_H
#define WFPROCEDURALDUNGEONENTRANCE_H


#include <cglm/cglm.h>

struct BinarySerializer;
struct Entity2D;
struct GameLayer2DData;

void WfInitProceduralDungeonEntrance();

void WfDeSerializeProceduralDungeonEntranceEntity(struct BinarySerializer* bs, struct Entity2D* pOutEnt, struct GameLayer2DData* pData);

void WfSerializeProceduralDungeonEntranceEntity(struct BinarySerializer* bs, struct Entity2D* pInEnt, struct GameLayer2DData* pData);


#endif