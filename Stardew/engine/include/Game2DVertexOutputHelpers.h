#ifndef GAME2DVERTEXOUTPUTHELPERS_H
#define GAME2DVERTEXOUTPUTHELPERS_H

#include <cglm/cglm.h>
#define VECTOR(a) a*
struct _AtlasSprite;
typedef struct _AtlasSprite AtlasSprite;
#include "DrawContext.h"

void OutputSpriteVerticesBase(
	AtlasSprite* pSprite,
	VECTOR(Worldspace2DVert)* pOutVert,
	VECTOR(VertIndexT)* pOutInd,
	VertIndexT* pNextIndex,
	vec2 tlPos,
	vec2 brPos
);

#endif