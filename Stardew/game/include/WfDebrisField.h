#ifndef WFDEBRISFIELD_H
#define WFDEBRIFIELD_H

struct BinarySerializer;
struct Entity2D;
struct GameLayer2DData;

void WfDeSerializeDebrisFieldEntity(struct BinarySerializer* bs, struct Entity2D* pOutEnt, struct GameLayer2DData* pData);

void WfSerializeDebrisFieldEntity(struct BinarySerializer* bs, struct Entity2D* pInEnt, struct GameLayer2DData* pData);

void WfDebrisFieldInit();

#endif