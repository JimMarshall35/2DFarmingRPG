#ifndef STATICCOLLIDERENTITY_H
#define STATICCOLLIDERENTITY_H

#include "Entities.h"
#include "HandleDefs.h"

struct Entity2DCollection;

struct EntitySerializerPair Et2D_Get2DRectStaticColliderSerializerPair();
struct EntitySerializerPair Et2D_Get2DCircleStaticColliderSerializerPair();

struct EntitySerializerPair Et2D_Get2DEllipseStaticColliderSerializerPair();
struct EntitySerializerPair Et2D_Get2DPolygonStaticColliderSerializerPair();

HEntity2D Et2D_AddRectangularStaticColliderEntity(struct Entity2DCollection* pEntities, float x, float y, float w, float h);
HEntity2D Et2D_AddCircularStaticColliderEntity(struct Entity2DCollection* pEntities, float x, float y, float r);

#endif