#include "SliderWidget.h"
#include "Widget.h"
#include "DataNode.h"
#include "XMLUIGameLayer.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "Atlas.h"
#include "AssertLib.h"
#include "StaticWidget.h"
#include "Atlas.h"
#include "WidgetVertexOutputHelpers.h"
#include "RootWidget.h"
#include "Scripting.h"
#include "Log.h"


float SliderWidget_GetWidth(struct SliderData* pData, struct WidgetPadding* pPadding)
{
	switch (pData->orientation)
	{
	case SO_Horizontal:
		return pData->lengthPx + pPadding->paddingLeft + pPadding->paddingRight;
	case SO_Vertical:
	{
		AtlasSprite* pSprite = At_GetSprite(pData->sliderStaticData.sprite, pData->sliderStaticData.atlas);
		return pSprite->widthPx * pData->sliderStaticData.scale.scaleX + pPadding->paddingLeft + pPadding->paddingRight;
	}
	default:
		EASSERT(false);
		break;
	}
	return 0.0f;
}

float SliderWidget_GetHeight(struct SliderData* pData, struct WidgetPadding* pPadding)
{
	switch (pData->orientation)
	{
	case SO_Horizontal:
	{
		AtlasSprite* pSprite = At_GetSprite(pData->sliderStaticData.sprite, pData->sliderStaticData.atlas);
		return pSprite->heightPx * pData->sliderStaticData.scale.scaleY + pPadding->paddingTop + pPadding->paddingBottom;
	}
	case SO_Vertical:
	{
		return pData->lengthPx + pPadding->paddingTop + pPadding->paddingBottom;
	}
	default:
		EASSERT(false);
		break;
	}

	return 0.0f;
}

static float GetWidth(struct UIWidget* pWidget, struct UIWidget* pParent)
{
	struct SliderData* pData = pWidget->pImplementationData;
	return SliderWidget_GetWidth(pData, &pWidget->padding);
}

static float GetHeight(struct UIWidget* pWidget, struct UIWidget* pParent)
{
	struct SliderData* pData = pWidget->pImplementationData;
	return SliderWidget_GetHeight(pData, &pWidget->padding);

}

static void LayoutChildren(struct UIWidget* pWidget, struct UIWidget* pParent)
{
}

static void OnDestroy(struct UIWidget* pWidget)
{
	free(pWidget->pImplementationData);
}

static void Populate3PanelRailQuads(float left, float top, WidgetQuad* pOutQuads, struct SliderData* pData)
{
	vec2 a2 = {
		left,
		top
	};
	
	switch (pData->orientation)
	{
	case SO_Vertical:
		{
			AtlasSprite* pSprite = At_GetSprite(pData->railStaticData.sprite, pData->railStaticData.atlas);
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
				pData->lengthPx - pSprite->heightPx / 3
			};
			
			TranslateWidgetQuad(trans, &pOutQuads[2]);
			pOutQuads[1].v[VL_TL].y = pOutQuads[0].v[VL_BL].y;
			pOutQuads[1].v[VL_TR].y = pOutQuads[0].v[VL_BR].y;
			pOutQuads[1].v[VL_BL].y = pOutQuads[2].v[VL_TL].y;
			pOutQuads[1].v[VL_BR].y = pOutQuads[2].v[VL_TR].y;
			for (int i = 0; i < 3; i++)
			{
				TranslateWidgetQuad(a2, &pOutQuads[i]);
			}
			break;
		}
	case SO_Horizontal:
		{
			AtlasSprite* pSprite = At_GetSprite(pData->railStaticData.sprite, pData->railStaticData.atlas);
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
				pData->lengthPx - pSprite->widthPx / 3,
				0
			};
		
			TranslateWidgetQuad(trans, &pOutQuads[2]);
			pOutQuads[1].v[VL_TL].x = pOutQuads[0].v[VL_TR].x;
			pOutQuads[1].v[VL_BL].x = pOutQuads[0].v[VL_BR].x;
			pOutQuads[1].v[VL_TR].x = pOutQuads[2].v[VL_TL].x;
			pOutQuads[1].v[VL_BR].x = pOutQuads[2].v[VL_BL].x;
			for (int i = 0; i < 3; i++)
			{
				TranslateWidgetQuad(a2, &pOutQuads[i]);
			}
			break;
		}
	}
}

static void PopulateSliderQuad(float left, float top, WidgetQuad* pOutQuad, struct SliderData* pData)
{
	//EASSERT(pData->fVal >= pData->fMinVal && pData->fVal <= pData->fMaxVal);
	AtlasSprite* pSprite = At_GetSprite(pData->sliderStaticData.sprite, pData->sliderStaticData.atlas);
	float fraction = (pData->fVal - pData->fMinVal) / (pData->fMaxVal - pData->fMinVal);
	PopulateWidgetQuadWholeSprite(pOutQuad, pSprite);
	vec2 t0 = {
		left,
		top
	};
	TranslateWidgetQuad(t0, pOutQuad);
	switch (pData->orientation)
	{
	case SO_Vertical:
	{
		float maxLen = pData->lengthPx - pSprite->heightPx;
		vec2 t = {
			0,
			maxLen* fraction
		};
		TranslateWidgetQuad(t, pOutQuad);
		break;
	}
	case SO_Horizontal:
	{
		float maxLen = pData->lengthPx - pSprite->widthPx;
		vec2 t = {
			maxLen* fraction,
			0
		};
		TranslateWidgetQuad(t, pOutQuad);
		break;
	}
	default:
		EASSERT(false);
		break;
	}
}

void* SliderWidget_OnOutputVerts(VECTOR(WidgetVertex) pOutVerts, struct SliderData* pData, float top, float left, struct WidgetPadding* pPadding)
{
	WidgetQuad rail[4];
	top = top + pPadding->paddingTop;
	left = left + pPadding->paddingLeft;
	Populate3PanelRailQuads(left, top, rail, pData);
	PopulateSliderQuad(left, top, &rail[3], pData);
	return OutputWidgetQuads(pOutVerts, rail, 4);
}

static void* OnOutputVerts(struct UIWidget* pWidget, VECTOR(WidgetVertex) pOutVerts)
{
	struct SliderData* pData = pWidget->pImplementationData;
	return SliderWidget_OnOutputVerts(pOutVerts, pData, pWidget->top, pWidget->left, &pWidget->padding);
}

static void OnPropertyChanged(struct UIWidget* pThisWidget, struct WidgetPropertyBinding* pBinding)
{
	struct SliderData* pData = pThisWidget->pImplementationData;
	if (strcmp(pBinding->boundPropertyName, "val") == 0)
	{
		char* fnName = UI_MakeBindingGetterFunctionName(pBinding->name);
		Sc_CallFuncInRegTableEntryTable(pThisWidget->scriptCallbacks.viewmodelTable, fnName, NULL, 0, 1);
		free(fnName);
		pData->fVal = Sc_Float();
		Sc_ResetStack();
		SetRootWidgetIsDirty(pData->rootWidget, true);
	}
}

void SliderWudget_SetSliderPositionFromMouse(struct UIWidget* pWidget, struct SliderData* pData, float x, float y, float top, float left, struct WidgetPadding padding)
{
	switch (pData->orientation)
	{
	case SO_Vertical:
	{
		float zeroY = top + padding.paddingTop;
		float maxY = top + padding.paddingTop + pData->lengthPx;
		float fraction = (y - zeroY) / (maxY - zeroY);
		pData->fVal = pData->fMinVal + fraction * (pData->fMaxVal - pData->fMinVal);
		break;
	}
	case SO_Horizontal:
	{
		float zeroX = left + padding.paddingLeft;
		float maxX = left + padding.paddingLeft + pData->lengthPx;
		float fraction = (x - zeroX) / (maxX - zeroX);
		pData->fVal = pData->fMinVal + fraction * (pData->fMaxVal - pData->fMinVal);
		break;
	}
	default:
		break;
	}
	struct WidgetPropertyBinding* pBinding = UI_FindBinding(pWidget, "val");
	if (pBinding)
	{
		char* setterName = UI_MakeBindingSetterFunctionName(pBinding->name);
		struct ScriptCallArgument arg;
		arg.type = SCA_number;
		arg.val.number = pData->fVal;
		Sc_CallFuncInRegTableEntryTable(pWidget->scriptCallbacks.viewmodelTable, setterName, &arg, 1, 0);
		free(setterName);

	}
	SetRootWidgetIsDirty(pData->rootWidget, true);
}

static void MouseButtonDownCallback(struct UIWidget* pWidget, float x, float y, int btn)
{
	struct SliderData* pData = pWidget->pImplementationData;
	pData->bMouseDown = true;
	SliderWudget_SetSliderPositionFromMouse(pWidget, pData, x, y, pWidget->top, pWidget->left, pWidget->padding);
}

static void MouseButtonUpCallback(struct UIWidget* pWidget, float x, float y, int btn)
{
	struct SliderData* pData = pWidget->pImplementationData;
	pData->bMouseDown = false;
}

static void MouseLeaveCallback(struct UIWidget* pWidget, float x, float y)
{
	struct SliderData* pData = pWidget->pImplementationData;
	pData->bMouseDown = false;
}

static void MouseMoveCallback(struct UIWidget* pWidget, float x, float y)
{
	struct SliderData* pData = pWidget->pImplementationData;
	if (pData->bMouseDown)
	{
		SliderWudget_SetSliderPositionFromMouse(pWidget, pData, x, y, pWidget->top, pWidget->left, pWidget->padding);
	}
}

static void PopulateStaticInternal(
	struct DataNode* pDataNode,
	struct StaticWidgetData* pWidgetData,
	struct XMLUIData* pUILayerData,
	const char* spriteAttribName,
	const char* scaleXAttribName,
	const char* scaleYAttribName)
{
	if(pDataNode->fnGetPropType(pDataNode, spriteAttribName) == DN_String)
	{
		size_t len = pDataNode->fnGetStrlen(pDataNode, spriteAttribName);
		if (pWidgetData->imageName)
		{
			free(pWidgetData->imageName);
		}
		pWidgetData->imageName = malloc(len + 1);
		pDataNode->fnGetStrcpy(pDataNode, spriteAttribName, pWidgetData->imageName);
		pWidgetData->sprite = At_FindSprite(pWidgetData->imageName, pUILayerData->atlas);
		pWidgetData->atlas = pUILayerData->atlas;
	}
	if(pDataNode->fnGetPropType(pDataNode, scaleXAttribName) == DN_Float)
	{
		pWidgetData->scale.scaleX = pDataNode->fnGetFloat(pDataNode, scaleXAttribName);
	}
	if(pDataNode->fnGetPropType(pDataNode, scaleYAttribName) == DN_Float)
	{
		pWidgetData->scale.scaleY = pDataNode->fnGetFloat(pDataNode, scaleYAttribName);
	}
}

static void ParseBindingEspressionAttribute(char* attribName, char* attribContent, struct UIWidget* pWidget, struct SliderData* pData, struct XMLUIData* pUILayerData)
{
	if (strcmp(attribName, "val") == 0)
	{
		UI_AddFloatPropertyBinding(pWidget, attribName, attribContent, &pData->fVal, pUILayerData->hViewModel);
	}
	else
	{
		Log_Error("invalid property binding: %s", attribContent);
	}

}

static void MakeSliderFromXML(struct UIWidget* pWidget, struct SliderData* pData, struct DataNode* pXMLNode, struct XMLUIData* pUILayerData)
{
	PopulateStaticInternal(pXMLNode, &pData->railStaticData, pUILayerData,
		"railSprite",
		"railScaleX",
		"railScaleY");
	PopulateStaticInternal(pXMLNode, &pData->sliderStaticData, pUILayerData,
		"sliderSprite",
		"sliderScaleX",
		"sliderScaleY");
	if(pXMLNode->fnGetPropType(pXMLNode, "orientation") == DN_String)
	{
		if(pXMLNode->fnContentStrCmp(pXMLNode, "horizontal"))
		{
			pData->orientation = SO_Horizontal;
		}
		else if (pXMLNode->fnContentStrCmp(pXMLNode, "vertical"))
		{
			pData->orientation = SO_Vertical;
		}
		else
		{
			Log_Warning("invalid slider orientation");
		}
	}
	if(pXMLNode->fnGetPropType(pXMLNode, "val") == DN_String)
	{
		size_t len = pXMLNode->fnGetStrlen(pXMLNode, "val");
		char* attribute = malloc(len + 1);
		pXMLNode->fnGetStrcpy(pXMLNode, "val", attribute);
		if(UI_IsAttributeStringABindingExpression(attribute))
		{
			ParseBindingEspressionAttribute("val", attribute, pWidget, pData, pUILayerData);
		}
		free(attribute);
	}
	if(pXMLNode->fnGetPropType(pXMLNode, "maxVal") == DN_Float)
	{
		pData->fMaxVal = pXMLNode->fnGetFloat(pXMLNode, "maxVal");
	}
	if(pXMLNode->fnGetPropType(pXMLNode, "minVal") == DN_Float)
	{
		pData->fMinVal = pXMLNode->fnGetFloat(pXMLNode, "minVal");
		pData->fVal = pData->fMinVal;
	}
}

void SliderWidget_MakeDefaultSliderWidget(struct SliderData* pData, struct XMLUIData* pUILayerData, enum SliderOrientation orientation)
{
	pData->orientation = orientation;
	pData->rootWidget = pUILayerData->rootWidget;

	// slider and rail sprites
	switch (pData->orientation)
	{
	case SO_Horizontal:
		{
			pData->sliderStaticData.imageName = malloc(strlen("defaultSliderHorizontal") + 1);
			strcpy(pData->sliderStaticData.imageName, "defaultSliderHorizontal");
			pData->railStaticData.imageName = malloc(strlen("defaultRailHorizontal") + 1);
			strcpy(pData->railStaticData.imageName, "defaultRailHorizontal");
			break;
		}
	case SO_Vertical:
	{
		pData->sliderStaticData.imageName = malloc(strlen("defaultSliderVertical") + 1);
		strcpy(pData->sliderStaticData.imageName, "defaultSliderVertical");
		pData->railStaticData.imageName = malloc(strlen("defaultRailVertical") + 1);
		strcpy(pData->railStaticData.imageName, "defaultRailVertical");
		break;
	}
	default:
		break;
	}


	pData->sliderStaticData.scale.scaleX = 1.0f;
	pData->sliderStaticData.scale.scaleY = 1.0f;
	pData->railStaticData.scale.scaleX = 1.0f;
	pData->railStaticData.scale.scaleY = 1.0f;

	pData->sliderStaticData.atlas = pUILayerData->atlas;
	pData->railStaticData.atlas = pUILayerData->atlas;

	pData->sliderStaticData.sprite = At_FindSprite(pData->sliderStaticData.imageName, pUILayerData->atlas);
	pData->railStaticData.sprite = At_FindSprite(pData->railStaticData.imageName, pUILayerData->atlas);

	// slider data
	pData->lengthPx = 200.0f;
	pData->fMaxVal = 200.0f;
	pData->fMinVal = 0.0f;
	pData->fVal = 0.0f;

}


static void MakeWidgetIntoSliderWidget(HWidget hWidget, struct DataNode* pDataNode, struct XMLUIData* pUILayerData)
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
	pWidget->fnOnBoundPropertyChanged = &OnPropertyChanged;
	pWidget->pImplementationData = malloc(sizeof(struct SliderData));

	pWidget->cCallbacks.Callbacks[WC_OnMouseDown].type = WC_OnMouseDown;
	pWidget->cCallbacks.Callbacks[WC_OnMouseDown].callback.mouseBtnFn = &MouseButtonDownCallback;
	
	pWidget->cCallbacks.Callbacks[WC_OnMouseUp].type = WC_OnMouseUp;
	pWidget->cCallbacks.Callbacks[WC_OnMouseUp].callback.mouseBtnFn = &MouseButtonUpCallback;

	pWidget->cCallbacks.Callbacks[WC_OnMouseLeave].type = WC_OnMouseLeave;
	pWidget->cCallbacks.Callbacks[WC_OnMouseLeave].callback.mousePosFn = &MouseLeaveCallback;

	pWidget->cCallbacks.Callbacks[WC_OnMouseMove].type = WC_OnMouseMove;
	pWidget->cCallbacks.Callbacks[WC_OnMouseMove].callback.mousePosFn = &MouseMoveCallback;

	memset(pWidget->pImplementationData, 0, sizeof(struct SliderData));
	SliderWidget_MakeDefaultSliderWidget(pWidget->pImplementationData, pUILayerData, SO_Horizontal);
	struct SliderData* pData = pWidget->pImplementationData;
	MakeSliderFromXML(pWidget, pData, pDataNode, pUILayerData);
}

HWidget SliderWidgetNew(HWidget hParent, struct DataNode* pDataNode, struct XMLUIData* pUILayerData)
{
	HWidget hWidget = UI_NewBlankWidget();
	MakeWidgetIntoSliderWidget(hWidget, pDataNode, pUILayerData);
	return hWidget;
}