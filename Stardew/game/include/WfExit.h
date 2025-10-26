#ifndef WFEXIT_H
#define WFEXIT_H

struct BinarySerializer;
struct Entity2D;
struct GameLayer2DData;

void WfInitExit();

void WfDeSerializeExitEntity(struct BinarySerializer* bs, struct Entity2D* pOutEnt, struct GameLayer2DData* pData);

void WfSerializeExitEntity(struct BinarySerializer* bs, struct Entity2D* pInEnt, struct GameLayer2DData* pData);

#endif