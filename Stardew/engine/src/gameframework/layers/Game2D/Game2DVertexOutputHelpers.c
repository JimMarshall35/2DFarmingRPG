#include "Game2DVertexOutputHelpers.h"
#include "Atlas.h"
#include "DynArray.h"
#include "DrawContext.h"
#include "Entities.h"
#include "Game2DLayer.h"
#include "GameFramework.h"
#include <string.h>


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
)
{
	VECTOR(Worldspace2DVert) outVert = *pOutVert;
	VECTOR(VertIndexT) outInd = *pOutInd;

	VertIndexT base = *pNextIndex;
	*pNextIndex += 4;
	Worldspace2DVert vert = {
		tlPos[0], tlPos[1],
		pSprite->topLeftUV_U, pSprite->topLeftUV_V
	};

	// top left
	VertIndexT tl = base;
	outVert = VectorPush(outVert, &vert);
	
	vert.x = trPos[0]; 
	vert.y = trPos[1];
	vert.u = pSprite->bottomRightUV_U;
	vert.v = pSprite->topLeftUV_V;
	
	// top right
	VertIndexT tr = base + 1;
	outVert = VectorPush(outVert, &vert);

	vert.x = blPos[0];
	vert.y = blPos[1];
	vert.u = pSprite->topLeftUV_U;
	vert.v = pSprite->bottomRightUV_V;

	// bottom left
	VertIndexT bl = base + 2;
	outVert = VectorPush(outVert, &vert);

	vert.x = brPos[0];
	vert.y = brPos[1];
	vert.u = pSprite->bottomRightUV_U;
	vert.v = pSprite->bottomRightUV_V;

	// bottom right
	VertIndexT br = base + 3;
	outVert = VectorPush(outVert, &vert);

	outInd = VectorPush(outInd, &tl);
	outInd = VectorPush(outInd, &tr);
	outInd = VectorPush(outInd, &bl);
	outInd = VectorPush(outInd, &tr);
	outInd = VectorPush(outInd, &br);
	outInd = VectorPush(outInd, &bl);

	*pOutVert = outVert;
	*pOutInd = outInd;
}

static void SpriteComp_GetBoundingBoxInternalBase(struct Entity2D* pEnt, hSprite sprite, struct GameFrameworkLayer* pLayer, vec2 outTL, vec2 outBR)
{
    struct GameLayer2DData* pLayerData = pLayer->userData;
    AtlasSprite* pSprite = At_GetSprite(sprite, pLayerData->hAtlas);
    vec2 tl = {0,0};
    vec2 offset = {pSprite->xOffsetToActual, pSprite->yOffsetToActual};
    glm_vec2_add(tl, offset, tl);

    vec2 br;
    vec2 size = {
        pSprite->actualWidthPX,
        pSprite->actualHeightPX
    };
    glm_vec2_add(tl, size, br);

	outTL[0] = tl[0];
	outTL[1] = tl[1];
	outBR[0] = br[0];
	outBR[1] = br[1];
}

void HSprite_DrawBase(
    hSprite spriteHandle,
    struct Transform2D* pSpriteTrans,
    struct Entity2D* pEnt, 
    struct GameFrameworkLayer* pLayer, 
    VECTOR(Worldspace2DVert)* outVerts, 
    VECTOR(VertIndexT)* outIndices, 
    VertIndexT* pNextIndex)
{
    vec3 tl, tr, bl, br;
    tl[2] = 1.0f;
    tr[2] = 1.0f;
    bl[2] = 1.0f;
    br[2] = 1.0f;

    SpriteComp_GetBoundingBoxInternalBase(pEnt, spriteHandle, pLayer, tl, br);
    tr[0] = br[0];
    tr[1] = tl[1];

    bl[0] = tl[0];
    bl[1] = br[1];

    struct GameLayer2DData* pLayerData = pLayer->userData;
    AtlasSprite* pSprite = At_GetSprite(spriteHandle, pLayerData->hAtlas);    
    

    mat3 t, sprite, ent;
    Et2D_Transform2DToMat3(pSpriteTrans, sprite);
    Et2D_Transform2DToMat3(&pEnt->transform, ent);
    glm_mat3_mul(sprite, ent, t);
    
    glm_mat3_mulv(t, tl, tl);
    glm_mat3_mulv(t, tr, tr);
    glm_mat3_mulv(t, bl, bl);
    glm_mat3_mulv(t, br, br);

	float sinT = sin(pSpriteTrans->rotation);
    float cosT = cos(pSpriteTrans->rotation);
    vec2 c;
	glm_vec2_add(pEnt->transform.position, pSpriteTrans->position, c);
    glm_vec2_add(c, pSpriteTrans->rotationPointRelative, c);
    mat3 rot = 
    {
        {                           cosT,                            sinT, 0},
        {                          -sinT,                            cosT, 0},
        {c[0] * (1 - cosT) + c[1] * sinT, c[1] * (1 - cosT) - c[0] * sinT, 1}
    };
    glm_mat3_mulv(rot, tl, tl);
    glm_mat3_mulv(rot, tr, tr);
    glm_mat3_mulv(rot, bl, bl);
    glm_mat3_mulv(rot, br, br);

    OutputSpriteVerticesBase(pSprite, outVerts, outIndices, pNextIndex, tl, tr, bl, br, &pEnt->transform);
}

