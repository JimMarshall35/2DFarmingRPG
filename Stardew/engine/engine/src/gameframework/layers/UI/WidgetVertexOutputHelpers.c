#include "WidgetVertexOutputHelpers.h"
#include <stdlib.h>
#include <string.h>
#include "AssertLib.h"
#include "DrawContext.h"
#include "Profile.h"

static bool bClipRegionSet = false;
GeomRect gClipRect = { 0,0,0,0 };

void SetWidgetQuadColour(WidgetQuad* pQuad, float r, float g, float b, float a)
{
	ZONE(ctx)
	for (int i = 0; i < 4; i++)
	{
		pQuad->v[i].r = r;
		pQuad->v[i].g = g;
		pQuad->v[i].b = b;
		pQuad->v[i].a = a;
	}
	ZONE_END(ctx)
}

void SetClipRect(GeomRect clipRect)
{
	memcpy(gClipRect, clipRect, sizeof(GeomRect));
	bClipRegionSet = true;
}

void UnsetClipRect()
{
	bClipRegionSet = false;
}

void PopulateWidgetQuadWholeSprite(WidgetQuad* pQuad, AtlasSprite* pSprt)
{
	ZONE(ctx)
	vec2 tl = { 0,0 };
	vec2 br = { pSprt->widthPx, pSprt->heightPx };
	PopulateWidgetQuad(pQuad, pSprt, tl, br);
	ZONE_END(ctx)
}

void PopulateWidgetQuad(WidgetQuad* pQuad, AtlasSprite* pSprt, vec2 subSpriteTL, vec2 subSpriteBR)
{
	ZONE(ctx)
	float subSpriteHeight = subSpriteBR[1] - subSpriteTL[1];
	float subSpriteWidth = subSpriteBR[0] - subSpriteTL[0];

	float uPerPxl = (pSprt->bottomRightUV_U - pSprt->topLeftUV_U) / (float)pSprt->widthPx;
	float vPerPxl = (pSprt->topLeftUV_V - pSprt->bottomRightUV_V) / (float)pSprt->heightPx;
	vec2 subSpriteTopLefUV = {
		pSprt->topLeftUV_U + subSpriteTL[0] * uPerPxl,
		pSprt->topLeftUV_V - subSpriteTL[1] * vPerPxl
	};
	vec2 subSpriteBottomRightUV = {
		subSpriteTopLefUV[0] + uPerPxl * subSpriteWidth,
		subSpriteTopLefUV[1] - vPerPxl * subSpriteHeight
	};

	pQuad->v[VL_TL].x = 0.0f;
	pQuad->v[VL_TL].y = 0.0f;
	pQuad->v[VL_TL].u = subSpriteTopLefUV[0];
	pQuad->v[VL_TL].v = subSpriteTopLefUV[1];
	pQuad->v[VL_TL].r = 1.0f; pQuad->v[VL_TL].g = 1.0f; pQuad->v[VL_TL].b = 1.0f; pQuad->v[VL_TL].a = 1.0f;

	pQuad->v[VL_TR].x = subSpriteWidth;
	pQuad->v[VL_TR].y = 0.0f;
	pQuad->v[VL_TR].u = subSpriteBottomRightUV[0];
	pQuad->v[VL_TR].v = subSpriteTopLefUV[1];
	pQuad->v[VL_TR].r = 1.0f; pQuad->v[VL_TR].g = 1.0f; pQuad->v[VL_TR].b = 1.0f; pQuad->v[VL_TR].a = 1.0f;

	pQuad->v[VL_BR].x = subSpriteWidth;
	pQuad->v[VL_BR].y = subSpriteHeight;
	pQuad->v[VL_BR].u = subSpriteBottomRightUV[0];
	pQuad->v[VL_BR].v = subSpriteBottomRightUV[1];
	pQuad->v[VL_BR].r = 1.0f; pQuad->v[VL_BR].g = 1.0f; pQuad->v[VL_BR].b = 1.0f; pQuad->v[VL_BR].a = 1.0f;

	pQuad->v[VL_BL].x = 0.0f;
	pQuad->v[VL_BL].y = subSpriteHeight;
	pQuad->v[VL_BL].u = subSpriteTopLefUV[0];
	pQuad->v[VL_BL].v = subSpriteBottomRightUV[1];
	pQuad->v[VL_BL].r = 1.0f; pQuad->v[VL_BL].g = 1.0f; pQuad->v[VL_BL].b = 1.0f; pQuad->v[VL_BL].a = 1.0f;
	ZONE_END(ctx)

}

static bool AllCornerOutsideOfRegion(WidgetQuad* pQuad)
{
	ZONE(ctx)
	int count = 0;
	for (int i = 0; i < VT_NUM; i++)
	{
		if (!Ge_PointInAABB(pQuad->v[i].x, pQuad->v[i].y, gClipRect))
		{
			count++;
		}
	}
	ZONE_END(ctx)
	return count == 4;
}

static float ClipUV(float p0, float p1, float uv0, float uv1, float midpos)
{
	//EASSERT(p1 > p0);
	//EASSERT(midpos >= p0 && midpos <= p1);
	ZONE(ctx)
	float s = p1 - p0;
	float s2 = midpos - p0;
	float t = s2 / s;
	float us = uv1 - uv0;
	ZONE_END(ctx)
	return uv0 + (us)*t;
}

static bool ClipQuad(WidgetQuad* pQuad)
{
	ZONE(ctx)
	if (AllCornerOutsideOfRegion(pQuad))
	{
		/*
			IF A QUAD IS GREATER THAN THE SIZE OF THE CANVAS IN BOTH DIMENSIONS IT WILL BE CLIPPED!!
		*/
		ZONE_END(ctx)
		return false;
	}
	for (int i = 0; i < VT_NUM; i++)
	{
		WidgetVertex* pVert = &pQuad->v[i];
		if (pVert->x < gClipRect[0])
		{
			EASSERT(i == VL_TL || i == VL_BL);
			pVert->u = ClipUV(pQuad->v[VL_TL].x, pQuad->v[VL_TR].x, pQuad->v[VL_TL].u, pQuad->v[VL_TR].u, gClipRect[0]);
			pVert->x = gClipRect[0];
		}
		if (pVert->x > gClipRect[2])
		{
			EASSERT(i == VL_TR || i == VL_BR);
			pVert->u = ClipUV(pQuad->v[VL_TL].x, pQuad->v[VL_TR].x, pQuad->v[VL_TL].u, pQuad->v[VL_TR].u, gClipRect[2]);
			pVert->x = gClipRect[2];
		}
		if (pVert->y < gClipRect[1])
		{
			EASSERT(i == VL_TL || i == VL_TR);
			pVert->v = ClipUV(pQuad->v[VL_TL].y, pQuad->v[VL_BL].y, pQuad->v[VL_TL].v, pQuad->v[VL_BL].v, gClipRect[1]);
			pVert->y = gClipRect[1];
		}
		if (pVert->y > gClipRect[3])
		{
			EASSERT(i == VL_BL || i == VL_BR);
			pVert->v = ClipUV(pQuad->v[VL_TL].y, pQuad->v[VL_BL].y, pQuad->v[VL_TL].v, pQuad->v[VL_BL].v, gClipRect[3]);
			pVert->y = gClipRect[3];
		}
	}
	ZONE_END(ctx)
	return true;
}

void* OutputWidgetQuad(VECTOR(WidgetVertex) pOutVerts, const WidgetQuad* pQuad)
{
	ZONE(ctx)
	WidgetQuad cpy;
	memcpy(&cpy, pQuad, sizeof(WidgetQuad));

	if (bClipRegionSet)
	{
		if (!ClipQuad(&cpy))
		{
			ZONE_END(ctx)
			return pOutVerts;
		}
	}

	const int numIndices = 6;
	const WidgetQuadVertexLocation indices[6] = {
		VL_TL,
		VL_TR,
		VL_BL,
		VL_TR,
		VL_BR,
		VL_BL
	};
	for (int i = 0; i < numIndices; i++)
	{
		int index = indices[i];
		pOutVerts = VectorPush(pOutVerts, &cpy.v[index]);
	}
	ZONE_END(ctx)
	return pOutVerts;
}

void* OutputWidgetQuads(VECTOR(WidgetVertex) pOutVerts, const WidgetQuad* pQuads, int num)
{
	ZONE(ctx)
	for (int i = 0; i < num; i++)
	{
		pOutVerts = OutputWidgetQuad(pOutVerts, &pQuads[i]);
	}
	ZONE_END(ctx)
	return pOutVerts;
}


void TranslateWidgetQuad(vec2 vector, WidgetQuad* pOutQuad)
{
	ZONE(ctx)
	for (int i = 0; i < 4; i++)
	{
		pOutQuad->v[i].x += vector[0];
		pOutQuad->v[i].y += vector[1];
	}
	ZONE_END(ctx)
}

// keeps TL where it is and sets a new size, expanding to the right and downwards
void SizeWidgetQuad(vec2 size, WidgetQuad* pOutQuad)
{
	ZONE(ctx)
	pOutQuad->v[VL_TR].x = pOutQuad->v[VL_TL].x + size[0];
	pOutQuad->v[VL_BR].x = pOutQuad->v[VL_BL].x + size[0];

	pOutQuad->v[VL_BL].y = pOutQuad->v[VL_TL].y + size[1];
	pOutQuad->v[VL_BR].y = pOutQuad->v[VL_TR].y + size[1];
	ZONE_END(ctx)
}

float WidgetQuadWidth(const WidgetQuad* pInQuad)
{
	return pInQuad->v[VL_TR].x - pInQuad->v[VL_TL].x;
}

float WidgetQuadHeight(const WidgetQuad* pInQuad)
{
	return pInQuad->v[VL_BR].y - pInQuad->v[VL_TR].y;
}

void ScaleWidgetQuad(float scaleX, float scaleY, WidgetQuad* pOutQuad)
{
	ZONE(ctx)
	float w = WidgetQuadWidth(pOutQuad);
	float h = WidgetQuadHeight(pOutQuad);
	float newW = w * scaleX;
	float newH = h * scaleY;
	vec2 sz = { newW, newH };
	SizeWidgetQuad(sz, pOutQuad);
	ZONE_END(ctx)
}


void ThreePanelScale(
	float left,
	float top,
	WidgetQuad* pOutQuads,
	enum WidgetHorizontalOrVertical orientation,
	float lengthPx,
	hSprite sprite,
	hAtlas atlas
)
{
	vec2 a2 = {
		left,
		top
	};
	
	switch (orientation)
	{
	case WI_Vertical:
		{
			AtlasSprite* pSprite = At_GetSprite(sprite, atlas);
			if(lengthPx >= (pSprite->heightPx / 3.0f) * 2)
			{
				vec2 tl = { 0,0 };
				vec2 br = { pSprite->widthPx, pSprite->heightPx / 3.0f };
				vec2 addition = { 0, pSprite->heightPx / 3.0f };
				PopulateWidgetQuad(&pOutQuads[0], pSprite, tl, br);
				glm_vec2_add(tl, addition, tl);
				glm_vec2_add(br, addition, br);
				PopulateWidgetQuad(&pOutQuads[1], pSprite, tl, br);
				glm_vec2_add(tl, addition, tl);
				glm_vec2_add(br, addition, br);
				PopulateWidgetQuad(&pOutQuads[2], pSprite, tl, br);
				
				vec2 trans = {
					0,
					lengthPx - pSprite->heightPx / 3
				};
				
				TranslateWidgetQuad(trans, &pOutQuads[2]);
				pOutQuads[1].v[VL_TL].y = pOutQuads[0].v[VL_BL].y;
				pOutQuads[1].v[VL_TR].y = pOutQuads[0].v[VL_BR].y;
				pOutQuads[1].v[VL_BL].y = pOutQuads[2].v[VL_TL].y;
				pOutQuads[1].v[VL_BR].y = pOutQuads[2].v[VL_TR].y;
				
			}
			else if(lengthPx > (pSprite->heightPx / 3.0f))
			{
				vec2 tl = { 0, pSprite->heightPx - lengthPx };
				vec2 br = { pSprite->widthPx, pSprite->heightPx / 3.0f };
				vec2 addition = { 0, pSprite->heightPx / 3.0f };
				PopulateWidgetQuad(&pOutQuads[0], pSprite, tl, br);

				tl[1] = br[1];

				PopulateWidgetQuad(&pOutQuads[1], pSprite, tl, br);

				br[1] += pSprite->heightPx / 3.0f;
				glm_vec2_add(tl, addition, tl);
				glm_vec2_add(br, addition, br);
				PopulateWidgetQuad(&pOutQuads[2], pSprite, tl, br);

				vec2 trans0 = {
					0,
					((pSprite->heightPx / 3) * 2) - lengthPx
				};
				TranslateWidgetQuad(trans0, &pOutQuads[0]);

				vec2 trans = {
					0,
					lengthPx - pSprite->heightPx / 3
				};
				
				TranslateWidgetQuad(trans, &pOutQuads[2]);

				pOutQuads[1].v[VL_TL].y = pOutQuads[0].v[VL_BL].y;
				pOutQuads[1].v[VL_TR].y = pOutQuads[0].v[VL_BR].y;
				pOutQuads[1].v[VL_BL].y = pOutQuads[2].v[VL_TL].y;
				pOutQuads[1].v[VL_BR].y = pOutQuads[2].v[VL_TR].y;


			}
			else
			{
				vec2 tl = { 0, pSprite->heightPx - lengthPx };
				vec2 br = { pSprite->widthPx, pSprite->heightPx - lengthPx };
				PopulateWidgetQuad(&pOutQuads[0], pSprite, tl, br);
				PopulateWidgetQuad(&pOutQuads[1], pSprite, tl, br);
				br[1] = pSprite->heightPx;
				PopulateWidgetQuad(&pOutQuads[2], pSprite, tl, br);

				vec2 trans = {
					0,
					lengthPx - pSprite->heightPx / 3
				};
				
				TranslateWidgetQuad(trans, &pOutQuads[2]);

				// pOutQuads[1].v[VL_TL].y = pOutQuads[0].v[VL_BL].y;
				// pOutQuads[1].v[VL_TR].y = pOutQuads[0].v[VL_BR].y;
				// pOutQuads[1].v[VL_BL].y = pOutQuads[2].v[VL_TL].y;
				// pOutQuads[1].v[VL_BR].y = pOutQuads[2].v[VL_TR].y;

			}
			
			break;
		}
	case WI_Horizontal:
		{
			/* TODO: Handle this as the vertical one is handled*/
			AtlasSprite* pSprite = At_GetSprite(sprite, atlas);
			vec2 tl = { 0,0 };
			vec2 br = { pSprite->widthPx/3, pSprite->heightPx };
			vec2 addition = { pSprite->widthPx / 3, 0 };
			PopulateWidgetQuad(&pOutQuads[0], pSprite, tl, br);
			glm_vec2_add(tl, addition, tl);
			glm_vec2_add(br, addition, br);
			PopulateWidgetQuad(&pOutQuads[1], pSprite, tl, br);
			glm_vec2_add(tl, addition, tl);
			glm_vec2_add(br, addition, br);
			PopulateWidgetQuad(&pOutQuads[2], pSprite, tl, br);
			vec2 trans = {
				lengthPx - pSprite->widthPx / 3,
				0
			};
		
			TranslateWidgetQuad(trans, &pOutQuads[2]);
			pOutQuads[1].v[VL_TL].x = pOutQuads[0].v[VL_TR].x;
			pOutQuads[1].v[VL_BL].x = pOutQuads[0].v[VL_BR].x;
			pOutQuads[1].v[VL_TR].x = pOutQuads[2].v[VL_TL].x;
			pOutQuads[1].v[VL_BR].x = pOutQuads[2].v[VL_BL].x;

			break;
		}
	}
	for (int i = 0; i < 3; i++)
	{
		TranslateWidgetQuad(a2, &pOutQuads[i]);
	}
}