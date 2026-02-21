#include "Game2DVertexOutputHelpers.h"
#include "Atlas.h"
#include "DynArray.h"
#include "DrawContext.h"
#include "Entities.h"

#include <immintrin.h>
#include <string.h>

//#define USE_SIMD_ROTATION

static void RotatePointAroundPoint(float x, float y, vec2 pivot, float rotation, vec2 outRotatedPoint)
{
	float xTemp = x - pivot[0];
	float yTemp = y - pivot[1];

	float xRot = xTemp * cos(rotation) - yTemp * sin(rotation);
	float yRot = xTemp * sin(rotation) + yTemp * cos(rotation);

	outRotatedPoint[0] = xRot + pivot[0];
	outRotatedPoint[1] = yRot + pivot[1];
}

static void RotateQuadAVX(
	float* tl_x, float* tl_y,
	float* tr_x, float* tr_y,
	float* bl_x, float* bl_y,
	float* br_x, float* br_y, 
	vec2 pivot, float rotation)
{
	// DOESN'T WORK YET
	__m256 _v = _mm256_set_ps(
		*tl_x, *tl_y,
		*tr_x, *tr_y,
		*bl_x, *bl_y,
		*br_x, *br_y
	);
	__m256 _pivot = _mm256_set_ps(
		pivot[0], pivot[1],
		pivot[0], pivot[1],
		pivot[0], pivot[1],
		pivot[0], pivot[1]
	);
	float cosR = cos(rotation);
	float sinR = sin(rotation);
	__m256 _cossin = _mm256_set_ps(
		cosR, sinR,
		cosR, sinR,
		cosR, sinR,
		cosR, sinR
	);
	__m256 _sincos = _mm256_set_ps(
		sinR, cosR,
		sinR, cosR,
		sinR, cosR,
		sinR, cosR
	);

	__m256 _rel = _mm256_sub_ps(_v, _pivot);

	__m256 _rel0 = _mm256_shuffle_ps(_rel, _rel, (0 | (0 << 2) | (2 << 4) | (2 << 6)));  // x0, x0, x1, x1, x2, x2 ...
	__m256 _rel1 = _mm256_shuffle_ps(_rel, _rel, (1 | (1 << 2) | (3 << 4) | (3 << 6)));  // y0, y0, y1, y1, y2, y2 ...

	__m256 _rel0CosSin = _mm256_mul_ps(_rel0, _cossin);
	__m256 _rel1SinCos = _mm256_mul_ps(_rel1, _sincos);

	__m256 _rel0CosSinMinusRel1SinCos = _mm256_sub_ps(_rel0CosSin, _rel1SinCos);
	__m256 _rel0CosSinAddRel1SinCos = _mm256_add_ps(_rel0CosSin, _rel1SinCos);

	float allset = 0;
	memset(&allset, 0xff, sizeof(float));
	__m256 _xMask = _mm256_set_ps(
		allset, 0,
		allset, 0,
		allset, 0,
		allset, 0
	);
	__m256 _yMask = _mm256_set_ps(
		0, allset,
		0, allset,
		0, allset,
		0, allset
	);
	__m256 _xPortion = _mm256_and_ps(_rel0CosSinMinusRel1SinCos, _xMask);
	__m256 _yPortion = _mm256_and_ps(_rel0CosSinAddRel1SinCos, _yMask);

	__m256 _rot = _mm256_or_ps(_xPortion, _yPortion);
	__m256 _rotated = _mm256_add_ps(_rot, _pivot);
	float vals[8];
	_mm256_storeu_ps(&vals[0], _rotated);

	*tl_x = vals[0];
	*tl_y = vals[1];
	*tr_x = vals[2];
	*tr_y = vals[3];
	*bl_x = vals[4];
	*bl_y = vals[5];
	*br_x = vals[6];
	*br_y = vals[7];
}

void OutputSpriteVerticesBase(
	AtlasSprite* pSprite,
	VECTOR(Worldspace2DVert)* pOutVert,
	VECTOR(VertIndexT)* pOutInd,
	VertIndexT* pNextIndex,
	vec2 tlPos,
	vec2 brPos,
	struct Transform2D* transform
)
{
	VECTOR(Worldspace2DVert) outVert = *pOutVert;
	VECTOR(VertIndexT) outInd = *pOutInd;

	VertIndexT base = *pNextIndex;
	*pNextIndex += 4;
	ivec2 dims = {
		brPos[0] - tlPos[0],
		brPos[1] - tlPos[1]
	};

	vec2 trPos = {brPos[0], tlPos[1]};
	vec2 blPos = {tlPos[0], brPos[1]};

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

	vec2 pivotPos;
	glm_vec2_add(transform->position, transform->rotationPointRelative, pivotPos);

#ifdef USE_SIMD_ROTATION
	RotateQuadAVX(
		&outVert[tl].x, &outVert[tl].y,
		&outVert[tr].x, &outVert[tr].y,
		&outVert[bl].x, &outVert[bl].y,
		&outVert[br].x, &outVert[br].y,
		pivotPos, transform->rotation);
#else
	vec2 pt;
	RotatePointAroundPoint(outVert[tl].x, outVert[tl].y, pivotPos, transform->rotation, pt);
	outVert[tl].x = pt[0]; outVert[tl].y = pt[1];
	RotatePointAroundPoint(outVert[tr].x, outVert[tr].y, pivotPos, transform->rotation, pt);
	outVert[tr].x = pt[0]; outVert[tr].y = pt[1];
	RotatePointAroundPoint(outVert[bl].x, outVert[bl].y, pivotPos, transform->rotation, pt);
	outVert[bl].x = pt[0]; outVert[bl].y = pt[1];
	RotatePointAroundPoint(outVert[br].x, outVert[br].y, pivotPos, transform->rotation, pt);
	outVert[br].x = pt[0]; outVert[br].y = pt[1];
#endif

	outInd = VectorPush(outInd, &tl);
	outInd = VectorPush(outInd, &tr);
	outInd = VectorPush(outInd, &bl);
	outInd = VectorPush(outInd, &tr);
	outInd = VectorPush(outInd, &br);
	outInd = VectorPush(outInd, &bl);

	*pOutVert = outVert;
	*pOutInd = outInd;
}