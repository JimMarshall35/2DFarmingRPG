#ifndef GEOMETRY_H
#define GEOMETRY_H
#include <stdbool.h>
#include <cglm/cglm.h>

enum Geom_RecCoords
{
	GR_TLX,
	GR_TLY,
	GR_BRX,
	GR_BRY,
};
typedef vec4 GeomRect;

bool Ge_PointInAABB(float pointX, float pointY, GeomRect rect);

bool Ge_AABBIntersect(vec2 tl1, vec2 br1, vec2 tl2, vec2 br2);

float Ge_AngleBetweenVec2s(vec2 a, vec2 b);

float Ge_DistanceBetweenPoints(vec2 a, vec2 b);

#endif