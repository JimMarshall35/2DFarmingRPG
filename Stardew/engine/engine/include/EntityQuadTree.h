#ifndef ENTITY2DQUADTREE_H
#define ENTITY2DQUADTREE_H
#include "HandleDefs.h"
#include "Entities.h"
#include <cglm/cglm.h>
#define VECTOR(a) a*

/**
 * @file EntityQuadTree.h
 * @brief static entities are stored in here for fast querying to be rendered
 * TODO: maybe rewrite this and replace with simpler binary space partioning
 * dynamic entities are stored in a simple linked list and are linearly AABBB checked against
 * the screen camera rectangle
 */

struct GameFrameworkLayer;

struct Entity2DCollection;

struct Entity2DQuadTreeInitArgs
{
    int x;
    int y;
    int w;
    int h;
};

void InitEntity2DQuadtreeSystem();

HEntity2DQuadtreeNode GetEntity2DQuadTree(struct Entity2DQuadTreeInitArgs* args);

void DestroyEntity2DQuadTree(HEntity2DQuadtreeNode quadTree);

HEntity2DQuadtreeEntityRef Entity2DQuadTree_Insert(struct Entity2DCollection* pCollection, HEntity2DQuadtreeNode quadTree, HEntity2D hEnt, struct GameFrameworkLayer* pLayer, int depth, int maxDepth);

void Entity2DQuadTree_Remove(HEntity2DQuadtreeNode quadTree, HEntity2DQuadtreeEntityRef ent);

VECTOR(HEntity2D) Entity2DQuadTree_Query(HEntity2DQuadtreeNode quadTree, vec2 regionTL, vec2 regionBR, VECTOR(HEntity2D) outEntities, struct Entity2DCollection* pCollection, struct GameFrameworkLayer* pLayer);

void Entity2DQuadTree_GetDims(HEntity2DQuadtreeNode quadTree, vec2 tl, float* w, float* h);

#endif

