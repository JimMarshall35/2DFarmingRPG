#include "BackgroundBoxWidget.h"
#include "XMLUIGameLayer.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "WidgetVertexOutputHelpers.h"
#include "AssertLib.h"
#include "Scripting.h"
#include "DataNode.h"
#include "Log.h"
#include "Profile.h"

static float GetWidth(struct UIWidget* pWidget, struct UIWidget* pParent)
{
	ZONE(ctx)
	EASSERT(UI_CountWidgetChildrenPtr(pWidget) == 1);
	if (pWidget->hFirstChild != NULL_HWIDGET)
	{
		struct UIWidget* pFirstChild = UI_GetWidget(pWidget->hFirstChild);
		float w = pFirstChild->fnGetWidth(pFirstChild, pWidget) + pWidget->padding.paddingLeft + pWidget->padding.paddingRight;
		ZONE_END(ctx)
		return w;
	}
	ZONE_END(ctx)
	return 0.0f;
}

static float GetHeight(struct UIWidget* pWidget, struct UIWidget* pParent)
{
	ZONE(ctx)
	EASSERT(UI_CountWidgetChildrenPtr(pWidget) == 1);
	if (pWidget->hFirstChild != NULL_HWIDGET)
	{
		struct UIWidget* pFirstChild = UI_GetWidget(pWidget->hFirstChild);
		float h = pFirstChild->fnGetHeight(pFirstChild, pWidget) + pWidget->padding.paddingTop + pWidget->padding.paddingBottom;
		ZONE_END(ctx)
		return h;
	}
	ZONE_END(ctx)
	return 0.0f;
}

static void LayoutChildren(struct UIWidget* pWidget, struct UIWidget* pParent)
{
	ZONE(ctx)
	struct UIWidget* pChild = UI_GetWidget(pWidget->hFirstChild);
	if (pChild)
	{
		pChild->top = pWidget->top + pWidget->padding.paddingTop;
		pChild->left = pWidget->left + pWidget->padding.paddingLeft;
		pChild->fnLayoutChildren(pChild, pWidget);
	}
	ZONE_END(ctx)
}

void BackgroundBoxWidget_Destroy(struct BackgroundBoxWidgetData* pBBoxData)
{
	ZONE(ctx)
	if (pBBoxData->imageName)
	{
		free(pBBoxData->imageName);
	}
	ZONE_END(ctx)
}


static void OnDestroy(struct UIWidget* pWidget)
{
	ZONE(ctx)
	struct BackgroundBoxWidgetData* pBBoxData = pWidget->pImplementationData;

	free(pWidget->pImplementationData);
	ZONE_END(ctx)
}

void* OnOutputVerts_3StripsHorizontal(
	struct BackgroundBoxWidgetData* pBBoxData,
	VECTOR(WidgetVertex) pOutVerts,
	float totalWidth,
	float totalHeight,
	const struct WidgetPadding* padding,
	float left, float top
)
{
	ZONE(ctx)
	AtlasSprite* pAtlasSprite = At_GetSprite(pBBoxData->sprite, pBBoxData->atlas);
	const struct WidgetScale* pScale = &pBBoxData->scale;
	WidgetQuad quads[3]; // in row first order, ie topleft, topmcentre, topright, middleleft, middlecentre, ect...
	float height = (float)pAtlasSprite->heightPx;
	float widthOver3 = (float)pAtlasSprite->widthPx / 3.0f;
	float widgetWidth = totalWidth - padding->paddingLeft - padding->paddingRight;//GetWidth(pWidget, NULL) - pWidget->padding.paddingLeft - pWidget->padding.paddingRight;
	float widgetHeight = totalHeight - padding->paddingTop - padding->paddingBottom;

	vec2 tl, br;
	tl[0] = 0.0f;
	tl[1] = 0.0f;
	br[0] = widthOver3;
	br[1] = height;
	for (int row = 0; row < 3; row++)
	{
		PopulateWidgetQuad(&quads[row], pAtlasSprite, tl, br);
		tl[0] += widthOver3;
		br[0] += widthOver3;
	}

	vec2 translation = { left + padding->paddingLeft, top + padding->paddingTop };
	for (int i = 0; i < 3; i++)
	{
		TranslateWidgetQuad(translation, &quads[i]);
	}

	vec2 size = {
		widthOver3 * pScale->scaleX,
		height * pScale->scaleY,

	};
	SizeWidgetQuad(size, &quads[0]);
	SizeWidgetQuad(size, &quads[1]);
	SizeWidgetQuad(size, &quads[2]);

	quads[1].v[VL_TR].x += widgetWidth - size[0];
	quads[1].v[VL_BR].x += widgetWidth - size[0];
	quads[1].v[VL_TL].x += size[0];
	quads[1].v[VL_BL].x += size[0];

	translation[1] = 0.0f;
	translation[0] = widgetWidth - size[0];
	TranslateWidgetQuad(translation, &quads[2]);
	for (int i = 0; i < 3; i++)
	{
		pOutVerts = OutputWidgetQuad(pOutVerts, &quads[i]);
	}
	ZONE_END(ctx)
	return pOutVerts;
}

void* OnOutputVerts_3StripsVertical(
	struct BackgroundBoxWidgetData* pBBoxData,
	VECTOR(WidgetVertex) pOutVerts,
	float totalWidth,
	float totalHeight,
	const struct WidgetPadding* padding,
	float left, float top
)
{
	return pOutVerts;
}

void* BackgroundBoxWidget_OutputVerts(
	struct BackgroundBoxWidgetData* pBBoxData,
	VECTOR(WidgetVertex) pOutVerts,
	float totalWidth,
	float totalHeight,
	const struct WidgetPadding* padding,
	float left, float top
)
{
	ZONE(ctx)
	AtlasSprite* pAtlasSprite = At_GetSprite(pBBoxData->sprite, pBBoxData->atlas);
	const struct WidgetScale* pScale = &pBBoxData->scale;
	float widgetWidth = totalWidth - padding->paddingLeft - padding->paddingRight;//GetWidth(pWidget, NULL) - pWidget->padding.paddingLeft - pWidget->padding.paddingRight;
	float widgetHeight = totalHeight - padding->paddingTop - padding->paddingBottom;

	if (widgetWidth <= pAtlasSprite->widthPx * pBBoxData->scale.scaleX && widgetHeight > pAtlasSprite->heightPx * pBBoxData->scale.scaleY)
	{
		void* p = OnOutputVerts_3StripsVertical(pBBoxData, pOutVerts, totalWidth, totalHeight, padding, left, top);
		ZONE_END(ctx)
		return p;
	}
	else if (widgetWidth > pAtlasSprite->widthPx * pBBoxData->scale.scaleX && widgetHeight <= pAtlasSprite->heightPx * pBBoxData->scale.scaleY)
	{
		void* p = OnOutputVerts_3StripsHorizontal(pBBoxData, pOutVerts, totalWidth, totalHeight, padding, left, top);
		ZONE_END(ctx)
		return p;
	}

	// widget is bigger in both dimensions than the sprite - do 9 panel scaling.
	/*
	 https://en.wikipedia.org/wiki/9-slice_scaling#:~:text=9%2Dslice%20scaling%20(also%20known,a%20grid%20of%20nine%20parts.
							________
		 _ _ _             |_|____|_|
		|_|_|_|   ->       | |    | |
		|_|_|_|            | |    | |
		|_|_|_|            |_|____|_|
						   |_|____|_|


	*/
	WidgetQuad quads[9]; // in row first order, ie topleft, topmcentre, topright, middleleft, middlecentre, ect...
	float widthOver3 = (float)pAtlasSprite->widthPx / 3.0f;
	float heightOver3 = (float)pAtlasSprite->heightPx / 3.0f;
	vec2 tl, br;

	tl[0] = 0;
	tl[1] = 0;
	br[0] = widthOver3;
	br[1] = heightOver3;
	int i = 0;
	for (int row = 0; row < 3; row++)
	{
		for (int col = 0; col < 3; col++)
		{
			PopulateWidgetQuad(&quads[i++], pAtlasSprite, tl, br);

			tl[0] += widthOver3;
			br[0] += widthOver3;
		}
		tl[0] = 0;
		br[0] = widthOver3;
		tl[1] += heightOver3;
		br[1] += heightOver3;
	}

	vec2 translation = { left + padding->paddingLeft, top + padding->paddingTop };
	for (int i = 0; i < 9; i++)
	{
		TranslateWidgetQuad(translation, &quads[i]);
	}

	vec2 size = {
		widthOver3 * pScale->scaleX,
		heightOver3 * pScale->scaleY,

	};
	SizeWidgetQuad(size, &quads[0]);

	// top row
	translation[0] = size[0];
	translation[1] = 0.0f;
	TranslateWidgetQuad(translation, &quads[1]);
	float borderPieceNewWidth = widgetWidth - size[0] * 2;
	vec2 horizontalBorderSize = { borderPieceNewWidth, size[1] };
	SizeWidgetQuad(horizontalBorderSize, &quads[1]);

	translation[0] = size[0] + borderPieceNewWidth;
	translation[1] = 0.0f;
	TranslateWidgetQuad(translation, &quads[2]);
	SizeWidgetQuad(size, &quads[2]);

	// middle row
	translation[0] = 0.0f;
	translation[1] = size[1];
	TranslateWidgetQuad(translation, &quads[3]);
	float borderPieceNewHeight = widgetHeight - size[1] * 2;
	vec2 verticalBorderSize = { size[0], borderPieceNewHeight };
	SizeWidgetQuad(verticalBorderSize, &quads[3]);

	translation[0] = size[0];
	translation[1] = size[1];
	TranslateWidgetQuad(translation, &quads[4]);
	vec2 middlePieceSize = { borderPieceNewWidth, borderPieceNewHeight };
	SizeWidgetQuad(middlePieceSize, &quads[4]);

	translation[0] = size[0] + borderPieceNewWidth;
	translation[1] = size[1];
	TranslateWidgetQuad(translation, &quads[5]);
	SizeWidgetQuad(verticalBorderSize, &quads[5]);

	// bottom row
	translation[0] = 0.0f;
	translation[1] = size[1] + borderPieceNewHeight;
	TranslateWidgetQuad(translation, &quads[6]);
	SizeWidgetQuad(size, &quads[6]);

	translation[0] = size[0];
	translation[1] = size[1] + borderPieceNewHeight;
	TranslateWidgetQuad(translation, &quads[7]);
	SizeWidgetQuad(horizontalBorderSize, &quads[7]);

	translation[0] = size[0] + borderPieceNewWidth;
	translation[1] = size[1] + borderPieceNewHeight;
	TranslateWidgetQuad(translation, &quads[8]);
	SizeWidgetQuad(size, &quads[8]);

	for (int i = 0; i < 9; i++)
	{
		pOutVerts = OutputWidgetQuad(pOutVerts, &quads[i]);
	}
	ZONE_END(ctx)

	return pOutVerts;
}

static void* OnOutputVerts(struct UIWidget* pWidget, VECTOR(WidgetVertex) pOutVerts)
{
	ZONE(ctx)
	float totalW = GetWidth(pWidget, NULL);
	float totalH = GetHeight(pWidget, NULL);
	pOutVerts = BackgroundBoxWidget_OutputVerts(
		pWidget->pImplementationData,
		pOutVerts,
		totalW,
		totalH,
		&pWidget->padding,
		pWidget->left,
		pWidget->top
	);
	pOutVerts = UI_Helper_OnOutputVerts(pWidget, pOutVerts);
	ZONE_END(ctx)
}


static void OnPropertyChanged(struct UIWidget* pThisWidget, struct WidgetPropertyBinding* pBinding)
{
	ZONE(ctx)
	struct BackgroundBoxWidgetData* pData = pThisWidget->pImplementationData;
	if (strcmp(pBinding->boundPropertyName, "sprite") == 0)
	{
		char* fnName = UI_MakeBindingGetterFunctionName(pBinding->name);
		Sc_CallFuncInRegTableEntryTable(pThisWidget->scriptCallbacks.viewmodelTable, fnName, NULL, 0, 1);
		free(fnName);
		EASSERT(pData->imageName);
		free(pData->imageName);
		size_t len = Sc_StackTopStringLen();
		pData->imageName = malloc(len + 1);
		Sc_StackTopStrCopy(pData->imageName);
		pData->sprite = At_FindSprite(pData->imageName, pData->atlas);
	}
	ZONE_END(ctx)
}

static void ParseBindingEspressionAttribute(char* pAttributeName, char* pAttributeContent, struct UIWidget* pWidget, struct BackgroundBoxWidgetData* pWidgetData, struct XMLUIData* pUILayerData)
{
	ZONE(ctx)
	if (strcmp(pAttributeName, "sprite") == 0)
	{
		if (pWidgetData->imageName)
		{
			free(pWidgetData->imageName);
			pWidgetData->imageName = NULL;
		}
		EASSERT(pWidgetData->imageName == NULL);
		UI_AddStringPropertyBinding(pWidget, pAttributeName, pAttributeContent, &pWidgetData->imageName, pUILayerData->hViewModel);
		pWidgetData->sprite = At_FindSprite(pWidgetData->imageName, pWidgetData->atlas);
	}
	else
	{
		Log_Warning("invalid property binding: %s", pAttributeContent);
	}
	ZONE_END(ctx)
}

static void ParseLiteralBackgroundBoxData(struct BackgroundBoxWidgetData* pWidgetData, const char* attributeContentBuffer)
{
	ZONE(ctx)
	if (pWidgetData->imageName)
	{
		free(pWidgetData->imageName);
		pWidgetData->imageName = NULL;
	}
	EASSERT(pWidgetData->imageName == NULL);
	pWidgetData->imageName = malloc(strlen(attributeContentBuffer) + 1);
	strcpy(pWidgetData->imageName, attributeContentBuffer);
	pWidgetData->sprite = At_FindSprite(pWidgetData->imageName, pWidgetData->atlas);
	ZONE_END(ctx)
}

void BackgroundBoxWidget_fromXML(struct UIWidget* pWidget, struct BackgroundBoxWidgetData* pWidgetData, struct DataNode* pDataNode, struct XMLUIData* pUILayerData)
{
	ZONE(ctx)
	pWidgetData->scale.scaleX = 1.0f;
	pWidgetData->scale.scaleY = 1.0f;

	pWidgetData->atlas = pUILayerData->atlas;
	
	if (pDataNode->fnGetPropType(pDataNode, "sprite") == DN_String)
	{
		size_t len = pDataNode->fnGetStrlen(pDataNode, "sprite");
		if(!len)
		{
			Log_Warning("empty string");
			ZONE_END(ctx)
			return;
		}
		char* val = malloc(len + 1);
		pDataNode->fnGetStrcpy(pDataNode, "sprite", val); 
		if (UI_IsAttributeStringABindingExpression(val))
		{
			ParseBindingEspressionAttribute("sprite", val, pWidget, pWidgetData, pUILayerData);
		}
		else
		{
			ParseLiteralBackgroundBoxData(pWidgetData, val);
		}
		free(val);
	}
	if( pDataNode->fnGetPropType(pDataNode, "scaleX") == DN_Float)
	{
		pWidgetData->scale.scaleX = pDataNode->fnGetFloat(pDataNode, "scaleX");
	}
	if( pDataNode->fnGetPropType(pDataNode, "scaleY") == DN_Float)
	{
		pWidgetData->scale.scaleY = pDataNode->fnGetFloat(pDataNode, "scaleY");
	}
	if (pWidgetData->imageName)
	{
		pWidgetData->sprite = At_FindSprite(pWidgetData->imageName, pUILayerData->atlas);
		pWidgetData->atlas = pUILayerData->atlas;
	}
	ZONE_END(ctx)
}

static void MakeWidgetIntoBackgroundBoxWidget(HWidget hWidget, struct DataNode* pXMLNode, struct XMLUIData* pUILayerData)
{
	ZONE(ctx)

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
	pWidget->fnOnBoundPropertyChanged = &OnPropertyChanged;
	pWidget->pImplementationData = malloc(sizeof(struct BackgroundBoxWidgetData));
	memset(pWidget->pImplementationData, 0, sizeof(struct BackgroundBoxWidgetData));

	struct BackgroundBoxWidgetData* pWidgetData = pWidget->pImplementationData;

	BackgroundBoxWidget_fromXML(pWidget, pWidgetData, pXMLNode, pUILayerData);
	ZONE_END(ctx)
}

HWidget BackgroundBoxWidgetNew(HWidget hParent, struct DataNode* pXMLNode, struct XMLUIData* pUILayerData)
{
	ZONE(ctx)
	HWidget hWidget = UI_NewBlankWidget();
	MakeWidgetIntoBackgroundBoxWidget(hWidget, pXMLNode, pUILayerData);
	ZONE_END(ctx)
	return hWidget;
}
