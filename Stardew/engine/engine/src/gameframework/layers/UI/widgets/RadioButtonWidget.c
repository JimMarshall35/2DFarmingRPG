#include "RadioButtonWidget.h"
#include "Widget.h"
#include "XMLUIGameLayer.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "Atlas.h"
#include "AssertLib.h"
#include "TextWidget.h"
#include "StaticWidget.h"
#include "RadioGroupWidget.h"
#include "DataNode.h"
#include "Log.h"

#ifndef max
#define max(a,b) (a>b?a:b)
#endif

enum RadioBtnPlacement
{
	RBP_Left,
	RBP_Right,
	RBP_Above,
	RBP_Below
};

struct RadioButtonData
{
	struct TextWidgetData textWidgetData;
	struct StaticWidgetData selectedRadioStatic;
	struct StaticWidgetData unSelectedRadioStatic;
	enum RadioBtnPlacement btnPlacementRelativeToText;
	float buttonDistanceFromText;
	bool bSelected;
};

static float GetBtnW(struct RadioButtonData* pData)
{
	AtlasSprite* pAtlasSprite = At_GetSprite(pData->bSelected ? pData->selectedRadioStatic.sprite : pData->unSelectedRadioStatic.sprite,
		pData->bSelected ? pData->selectedRadioStatic.atlas : pData->unSelectedRadioStatic.atlas);
	return pAtlasSprite->widthPx;
}

static float GetBtnH(struct RadioButtonData* pData)
{
	AtlasSprite* pAtlasSprite = At_GetSprite(pData->bSelected ? pData->selectedRadioStatic.sprite : pData->unSelectedRadioStatic.sprite,
		pData->bSelected ? pData->selectedRadioStatic.atlas : pData->unSelectedRadioStatic.atlas);
	return pAtlasSprite->heightPx;
}


static void GetSpriteAndAtlas(struct RadioButtonData* pData, hSprite* outSprite, hAtlas* outAtlas)
{
	if (pData->bSelected)
	{
		*outAtlas = pData->selectedRadioStatic.atlas;
		*outSprite = pData->selectedRadioStatic.sprite;
	}
	else
	{
		*outAtlas = pData->unSelectedRadioStatic.atlas;
		*outSprite = pData->unSelectedRadioStatic.sprite;
	}
}

static float GetWidth(struct UIWidget* pWidget, struct UIWidget* pParent)
{
	struct RadioButtonData* pData = pWidget->pImplementationData;
	hSprite sprite;
	hAtlas atlas;
	GetSpriteAndAtlas(pData, &sprite, &atlas);
	AtlasSprite* pAtlasSprite = At_GetSprite(sprite, atlas);
	float strW = Fo_StringWidth(pData->textWidgetData.atlas, pData->textWidgetData.font, pData->textWidgetData.content);
	float btnW = GetBtnW(pData);
	switch (pData->btnPlacementRelativeToText)
	{
	case RBP_Above:
	case RBP_Below:
		return max(strW, btnW)
			+ pWidget->padding.paddingLeft
			+ pWidget->padding.paddingRight;
		break;
	case RBP_Left:
	case RBP_Right:
		return 
			strW
			+ pData->buttonDistanceFromText 
			+ btnW
			+ pWidget->padding.paddingLeft 
			+ pWidget->padding.paddingRight;
		break;
	}
	return 0.0f;
}

static float GetHeight(struct UIWidget* pWidget, struct UIWidget* pParent)
{
	struct RadioButtonData* pData = pWidget->pImplementationData;
	hSprite sprite;
	hAtlas atlas;
	GetSpriteAndAtlas(pData, &sprite, &atlas);
	AtlasSprite* pAtlasSprite = At_GetSprite(sprite, atlas);
	float strH = Fo_StringHeight(pData->textWidgetData.atlas, pData->textWidgetData.font, pData->textWidgetData.content);
	float btnH = GetBtnH(pData);

	switch (pData->btnPlacementRelativeToText)
	{
	case RBP_Above:
	case RBP_Below:
		return strH +
			pData->buttonDistanceFromText +
			btnH +
			pWidget->padding.paddingTop +
			pWidget->padding.paddingBottom;
		break;
	case RBP_Left:
	case RBP_Right:
		return max(strH, btnH) + pWidget->padding.paddingTop + pWidget->padding.paddingBottom;
		break;
	}
	return 0.0f;
}

static void LayoutChildren(struct UIWidget* pWidget, struct UIWidget* pParent)
{
}

static void OnDestroy(struct UIWidget* pWidget)
{
	struct RadioButtonData* pData = pWidget->pImplementationData;
	StaticWidget_Destroy(&pData->selectedRadioStatic);
	StaticWidget_Destroy(&pData->unSelectedRadioStatic);
	TextWidget_Destroy(&pData->textWidgetData);
	free(pWidget->pImplementationData);
}

static void* OnOutputVerts(struct UIWidget* pWidget, VECTOR(WidgetVertex) pOutVerts)
{
	struct RadioButtonData* pData = pWidget->pImplementationData;

	float startL = pWidget->left + pWidget->padding.paddingLeft;
	float startT = pWidget->top + pWidget->padding.paddingTop;
	struct WidgetPadding p;
	memset(&p, 0, sizeof(struct WidgetPadding));

	switch (pData->btnPlacementRelativeToText)
	{
	case RBP_Right:
		{
			pOutVerts = TextWidget_OutputVerts(startL, startT, &p, &pData->textWidgetData, pOutVerts);
			float strW = Fo_StringWidth(pData->textWidgetData.atlas, pData->textWidgetData.font, pData->textWidgetData.content);
			startL += strW + pData->buttonDistanceFromText;
			struct StaticWidgetData* pStaticWidgetData = pData->bSelected ? &pData->selectedRadioStatic : &pData->unSelectedRadioStatic;
			pOutVerts = StaticWidget_OnOutputVerts(pStaticWidgetData, startL, startT, &p, pOutVerts);
			break;
		}
	case RBP_Left:
		{
			struct StaticWidgetData* pStaticWidgetData = pData->bSelected ? &pData->selectedRadioStatic : &pData->unSelectedRadioStatic;
			pOutVerts = StaticWidget_OnOutputVerts(pStaticWidgetData, startL, startT, &p, pOutVerts);
			float bw = GetBtnW(pData);
			startL += bw + pData->buttonDistanceFromText;
			pOutVerts = TextWidget_OutputVerts(startL, startT, &p, &pData->textWidgetData, pOutVerts);

			break;
		}
	case RBP_Above:
		{
			float wWidth = GetWidth(pWidget, NULL);
			float bw = GetBtnW(pData);
			float bh = GetBtnH(pData);
			float oldL = startL;
			startL += (wWidth / 2.0f) - (bw / 2.0f);
			struct StaticWidgetData* pStaticWidgetData = pData->bSelected ? &pData->selectedRadioStatic : &pData->unSelectedRadioStatic;
			pOutVerts = StaticWidget_OnOutputVerts(pStaticWidgetData, startL, startT, &p, pOutVerts);
			startL = oldL;
			startT += bh + pData->buttonDistanceFromText;
			pOutVerts = TextWidget_OutputVerts(startL, startT, &p, &pData->textWidgetData, pOutVerts);
			break;
		}
	case RBP_Below:
		{
			pOutVerts = TextWidget_OutputVerts(startL, startT, &p, &pData->textWidgetData, pOutVerts);
			float th = Fo_StringHeight(pData->textWidgetData.atlas, pData->textWidgetData.font, pData->textWidgetData.content);
			startT += th;
			float bw = GetBtnW(pData);
			float wWidth = GetWidth(pWidget, NULL);
			startL += (wWidth / 2.0f) - (bw / 2.0f);
			startT += pData->buttonDistanceFromText;
			struct StaticWidgetData* pStaticWidgetData = pData->bSelected ? &pData->selectedRadioStatic : &pData->unSelectedRadioStatic;
			pOutVerts = StaticWidget_OnOutputVerts(pStaticWidgetData, startL, startT, &p, pOutVerts);
			break;
		}
	}
	

	return pOutVerts;
}

static void OnPropertyChanged(struct UIWidget* pThisWidget, struct WidgetPropertyBinding* pBinding)
{

}


static void MouseButtonDownCallback(struct UIWidget* pWidget, float x, float y, int btn)
{
	RadioGroup_ChildSelected(pWidget->hParent, pWidget);

}

static void MouseButtonUpCallback(struct UIWidget* pWidget, float x, float y, int btn)
{
	
}

static void MouseLeaveCallback(struct UIWidget* pWidget, float x, float y)
{
	
}

static void MouseMoveCallback(struct UIWidget* pWidget, float x, float y)
{
	
}

static void MakeDefaultRadioButtonData(struct RadioButtonData* pData, struct XMLUIData* pUILayerData)
{
	pData->bSelected = false;
	pData->btnPlacementRelativeToText = RBP_Right;
	pData->buttonDistanceFromText = 10.0f;

	pData->textWidgetData.atlas = pUILayerData->atlas;
	pData->textWidgetData.content = malloc(strlen("option") + 1);
	strcpy(pData->textWidgetData.content, "option");
	pData->textWidgetData.fSizePts = 32.0f;
	pData->textWidgetData.font = Fo_FindFont(pData->textWidgetData.atlas, "default", 32.0f);
	pData->textWidgetData.r = 0.0f;
	pData->textWidgetData.g = 0.0f;
	pData->textWidgetData.b = 0.0f;
	pData->textWidgetData.a = 1.0f;

	pData->selectedRadioStatic.imageName = malloc(strlen("radioChecked") + 1);
	strcpy(pData->selectedRadioStatic.imageName, "radioChecked");
	pData->selectedRadioStatic.atlas = pUILayerData->atlas;
	pData->selectedRadioStatic.scale.scaleX = 1.0f;
	pData->selectedRadioStatic.scale.scaleY = 1.0f;
	pData->selectedRadioStatic.sprite = At_FindSprite(pData->selectedRadioStatic.imageName, pData->selectedRadioStatic.atlas);

	pData->unSelectedRadioStatic.imageName = malloc(strlen("radioUnchecked") + 1);
	strcpy(pData->unSelectedRadioStatic.imageName, "radioUnchecked");
	pData->unSelectedRadioStatic.atlas = pUILayerData->atlas;
	pData->unSelectedRadioStatic.scale.scaleX = 1.0f;
	pData->unSelectedRadioStatic.scale.scaleY = 1.0f;
	pData->unSelectedRadioStatic.sprite = At_FindSprite(pData->unSelectedRadioStatic.imageName, pData->unSelectedRadioStatic.atlas);
}

static void RadioButtonWidget_MakeFromXML(struct DataNode* pDataNode, struct RadioButtonData* pData)
{
	if(pDataNode->fnGetPropType(pDataNode, "btnPlacement") == DN_String)
	{
		if (pDataNode->fnStrCmp(pDataNode, "btnPlacement", "Left"))
		{
			pData->btnPlacementRelativeToText = RBP_Left;
		}
		else if (pDataNode->fnStrCmp(pDataNode, "btnPlacement", "Right"))
		{
			pData->btnPlacementRelativeToText = RBP_Right;
		}
		else if (pDataNode->fnStrCmp(pDataNode, "btnPlacement", "Above"))
		{
			pData->btnPlacementRelativeToText = RBP_Above;
		}
		else if (pDataNode->fnStrCmp(pDataNode, "btnPlacement", "Below"))
		{
			pData->btnPlacementRelativeToText = RBP_Below;
		}
		else
		{
			char* val = malloc(pDataNode->fnGetStrlen(pDataNode, "btnPlacement") + 1);
			pDataNode->fnGetStrcpy(pDataNode, "btnPlacement", val);
			Log_Error("Invlaid RadioBtnPlacement val '%s'", val);
			free(val);
		}
	}
}

static void MakeWidgetIntoRadioButtonWidget(HWidget hWidget, struct DataNode* pDataNode, struct XMLUIData* pUILayerData)
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
	pWidget->pImplementationData = malloc(sizeof(struct RadioButtonData));

	pWidget->cCallbacks.Callbacks[WC_OnMouseDown].type = WC_OnMouseDown;
	pWidget->cCallbacks.Callbacks[WC_OnMouseDown].callback.mouseBtnFn = &MouseButtonDownCallback;
	
	pWidget->cCallbacks.Callbacks[WC_OnMouseUp].type = WC_OnMouseUp;
	pWidget->cCallbacks.Callbacks[WC_OnMouseUp].callback.mouseBtnFn = &MouseButtonUpCallback;

	pWidget->cCallbacks.Callbacks[WC_OnMouseLeave].type = WC_OnMouseLeave;
	pWidget->cCallbacks.Callbacks[WC_OnMouseLeave].callback.mousePosFn = &MouseLeaveCallback;

	pWidget->cCallbacks.Callbacks[WC_OnMouseMove].type = WC_OnMouseMove;
	pWidget->cCallbacks.Callbacks[WC_OnMouseMove].callback.mousePosFn = &MouseMoveCallback;

	memset(pWidget->pImplementationData, 0, sizeof(struct RadioButtonData));
	struct RadioButtonData* pData = pWidget->pImplementationData;

	MakeDefaultRadioButtonData(pData, pUILayerData);

	StaticWidget_MakeFromXML(&pData->selectedRadioStatic, pDataNode, pUILayerData);
	StaticWidget_MakeFromXML(&pData->unSelectedRadioStatic, pDataNode, pUILayerData);
	TextWidget_FromXML(pWidget, &pData->textWidgetData, pDataNode, pUILayerData);
	RadioButtonWidget_MakeFromXML(pDataNode, pData);
}

HWidget RadioButtonWidgetNew(HWidget hParent, struct DataNode* pDataNode, struct XMLUIData* pUILayerData)
{
	HWidget hWidget = UI_NewBlankWidget();
	MakeWidgetIntoRadioButtonWidget(hWidget, pDataNode, pUILayerData);
	return hWidget;
}

void RadioButton_SetSelected(HWidget hRadioButton, bool bSelected)
{
	struct UIWidget* pWidget = UI_GetWidget(hRadioButton);
	struct RadioButtonData* pData = pWidget->pImplementationData;
	pData->bSelected = bSelected;
}
