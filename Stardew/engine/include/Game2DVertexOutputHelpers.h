#ifndef GAME2DVERTEXOUTPUTHELPERS_H
#define GAME2DVERTEXOUTPUTHELPERS_H

#include <cglm/cglm.h>
#define VECTOR(a) a*
struct _AtlasSprite;
typedef struct _AtlasSprite AtlasSprite;
#include "DrawContext.h"

struct Transform2D; 
struct Entity2D;
struct GameFrameworkLayer;

void OutputSpriteVerticesBase(
	AtlasSprite* pSprite,
	VECTOR(Worldspace2DVert)* pOutVert,
	VECTOR(VertIndexT)* pOutInd,
	VertIndexT* pNextIndex,
	vec2 tlPos,
	vec2 trPos,
	vec2 blPos,
	vec2 brPos,
	struct Transform2D* transform
);

void HSprite_DrawBase(
    hSprite sprite,
    struct Transform2D* pSpriteTrans,
    struct Entity2D* pEnt, 
    struct GameFrameworkLayer* pLayer, 
    VECTOR(Worldspace2DVert)* outVerts, 
    VECTOR(VertIndexT)* outIndices, 
    VertIndexT* pNextIndex);

void RotatePointAroundPoint(float x, float y, vec2 pivot, float rotation, vec2 outRotatedPoint);

#endif