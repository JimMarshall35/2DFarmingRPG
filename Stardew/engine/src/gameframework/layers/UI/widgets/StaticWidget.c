#include "StaticWidget.h"
#include "XMLUIGameLayer.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "Atlas.h"
#include "AssertLib.h"
#include "WidgetVertexOutputHelpers.h"
#include "DataNode.h"
#include "RootWidget.h"


float StaticWidget_GetWidth(struct StaticWidgetData* pStaticData, struct WidgetPadding* pPadding)
{
	AtlasSprite* pAtlasSprite = At_GetSprite(pStaticData->sprite, pStaticData->atlas);
	if (!pAtlasSprite)
	{
		return 0;
	}
	return pStaticData->scale.scaleX * (pAtlasSprite->widthPx + pPadding->paddingLeft + pPadding->paddingRight);
}

float StaticWidget_GetHeight(struct StaticWidgetData* pStaticData, struct WidgetPadding* pPadding)
{
	AtlasSprite* pAtlasSprite = At_GetSprite(pStaticData->sprite, pStaticData->atlas);
	if (!pAtlasSprite)
	{
		return 0;
	}

	return  pStaticData->scale.scaleY * (pAtlasSprite->heightPx + pPadding->paddingTop + pPadding->paddingBottom);
}

void StaticWidget_Destroy(struct StaticWidgetData* pStaticData)
{
	assert(pStaticData->imageName);
	free(pStaticData->imageName);
}

void* StaticWidget_OnOutputVerts(struct StaticWidgetData* pStaticData, float left, float top, struct WidgetPadding* pPadding, VECTOR(WidgetVertex) pOutVerts)
{
	AtlasSprite* pSprite = At_GetSprite(pStaticData->sprite, pStaticData->atlas);
	if(!pSprite)
	{
		return pOutVerts; /* sometimes hit for some reason */
	}
	float width = pSprite->widthPx;
	float height = pSprite->heightPx;

	WidgetQuad quad;
	vec2 translate = {
		left + pPadding->paddingLeft,
		top + pPadding->paddingTop
	};
	PopulateWidgetQuadWholeSprite(&quad, pSprite);
	TranslateWidgetQuad(translate, &quad);
	ScaleWidgetQuad(pStaticData->scale.scaleX, pStaticData->scale.scaleY, &quad);
	pOutVerts = OutputWidgetQuad(pOutVerts, &quad);
	return pOutVerts;
}

void StaticWidget_MakeFromXML(struct StaticWidgetData* pWidgetData, struct DataNode* pDataNode, struct XMLUIData* pUILayerData)
{
	if(pDataNode->fnGetPropType(pDataNode, "sprite") == DN_String)
	{
		size_t len = pDataNode->fnGetStrlen(pDataNode, "sprite");
		if (pWidgetData->imageName)
		{
			free(pWidgetData->imageName);
		}
		pWidgetData->imageName = malloc(len + 1);
		pDataNode->fnGetStrcpy(pDataNode, "sprite", pWidgetData->imageName);
	}
	if(pDataNode->fnGetPropType(pDataNode, "scaleX") == DN_Float)
	{
		pWidgetData->scale.scaleX = pDataNode->fnGetFloat(pDataNode, "scaleX");
	}
	if(pDataNode->fnGetPropType(pDataNode, "scaleY") == DN_Float)
	{
		pWidgetData->scale.scaleY = pDataNode->fnGetFloat(pDataNode, "scaleY");
	}
	if (pWidgetData->imageName)
	{
		pWidgetData->sprite = At_FindSprite(pWidgetData->imageName, pUILayerData->atlas);
		pWidgetData->atlas = pUILayerData->atlas;
	}
}

static float GetWidth(struct UIWidget* pWidget, struct UIWidget* pParent)
{
	struct StaticWidgetData* pStaticData = pWidget->pImplementationData;
	return StaticWidget_GetWidth(pStaticData, &pWidget->padding);
}

static float GetHeight(struct UIWidget* pWidget, struct UIWidget* pParent)
{
	struct StaticWidgetData* pStaticData = pWidget->pImplementationData;
	return StaticWidget_GetHeight(pStaticData, &pWidget->padding);
}

static void LayoutChildren(struct UIWidget* pWidget, struct UIWidget* pParent)
{
	RootWidget_LayoutChildren(pWidget, pParent, 0, 0);
}

static void OnDestroy(struct UIWidget* pWidget)
{
	StaticWidget_Destroy(pWidget->pImplementationData);
	free(pWidget->pImplementationData);
}

static void* OnOutputVerts(struct UIWidget* pWidget, VECTOR(WidgetVertex) pOutVerts)
{
	struct StaticWidgetData* pStaticData = pWidget->pImplementationData;
	pOutVerts = StaticWidget_OnOutputVerts(pStaticData, pWidget->left, pWidget->top, &pWidget->padding, pOutVerts);
	pOutVerts = UI_Helper_OnOutputVerts(pWidget, pOutVerts);
	return pOutVerts;
}

static void MakeWidgetIntoStatic(HWidget hWidget, struct DataNode* pXMLNode, struct XMLUIData* pUILayerData)
{
	struct UIWidget* pWidget = UI_GetWidget(hWidget);

	pWidget->hNext = -1;
	pWidget->hPrev = -1;
	pWidget->hParent = -1;
	pWidget->hFirstChild = -1;
	pWidget->fnGetHeight = &GetHeight;
	pWidget->fnGetWidth = &GetWidth;
	pWidget->fnLayoutChildren = &LayoutChildren;
	pWidget->fnOnDestroy = &OnDestroy;
	pWidget->fnOutputVertices = &OnOutputVerts;
	pWidget->pImplementationData = malloc(sizeof(struct StaticWidgetData));
	struct StaticWidgetData* pWidgetData = pWidget->pImplementationData;
	memset(pWidgetData, 0, sizeof(struct StaticWidgetData));

	pWidgetData->scale.scaleX = 1.0f;
	pWidgetData->scale.scaleY = 1.0f;
	StaticWidget_MakeFromXML(pWidgetData, pXMLNode, pUILayerData);

	pWidget->width.type = WD_Auto;
	pWidget->height.type = WD_Auto;
}

HWidget StaticWidgetNew(HWidget hParent, struct DataNode* pXMLNode, struct XMLUIData* pUILayerData)
{
	HWidget hWidget = UI_NewBlankWidget();
	MakeWidgetIntoStatic(hWidget, pXMLNode, pUILayerData);
	return hWidget;
}
