#ifndef WIDGETVERTEXOUTPUTHELPERS_H
#define WIDGETVERTEXOUTPUTHELPERS_H
#include "Widget.h"
#include <cglm/cglm.h>
#include "Atlas.h"

typedef enum
{
	VL_TL,
	VL_TR,
	VL_BL,
	VL_BR,
	VT_NUM
}WidgetQuadVertexLocation;

enum WidgetHorizontalOrVertical
{
	WI_Horizontal,
	WI_Vertical
};


struct Vert2DColourTexture;
typedef struct Vert2DColourTexture WidgetVertex;
struct Vert2DColourTextureQuad;
typedef struct Vert2DColourTextureQuad WidgetQuad;


void PopulateWidgetQuadWholeSprite(WidgetQuad* pQuad, AtlasSprite* pSprt);
void PopulateWidgetQuad(WidgetQuad* pQuad, AtlasSprite* pSprt, vec2 subSpriteTL, vec2 subSpriteBR);
void* OutputWidgetQuad(VECTOR(WidgetVertex) pOutVerts, const WidgetQuad* pQuad);
void* OutputWidgetQuads(VECTOR(WidgetVertex) pOutVerts, const WidgetQuad* pQuad, int num);
void TranslateWidgetQuad(vec2 vector, WidgetQuad* pOutQuad);
void SizeWidgetQuad(vec2 size, WidgetQuad* pOutQuad);
float WidgetQuadWidth(const WidgetQuad* pInQuad);
float WidgetQuadHeight(const WidgetQuad* pInQuad);
void ScaleWidgetQuad(float scaleX, float scaleY, WidgetQuad* pOutQuad);
void SetWidgetQuadColour(WidgetQuad* pQuad, float r, float g, float b, float a);

/// @brief clip any polys on output to be within this region
/// @param clipRect
void SetClipRect(GeomRect clipRect);
void UnsetClipRect();



void ThreePanelScale(
	float left,
	float top,
	WidgetQuad* pOutQuads,
	enum WidgetHorizontalOrVertical orientation,
	float lengthPx,
	hSprite sprite,
	hAtlas atlas
);

#endif