#include "TextButtonWidget.h"
#include "Widget.h"
#include "XMLUIGameLayer.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "Atlas.h"
#include "AssertLib.h"
#include "TextWidget.h"
#include "BackgroundBoxWidget.h"
#include "RootWidget.h"
#include "Scripting.h"
#include "DataNode.h"

enum ButtonType
{
	BT_FireOnRelease,
	BT_FireOnDown
};

struct TextButtonWidgetData
{
	struct TextWidgetData textWidgetData;
	struct BackgroundBoxWidgetData backgroundBoxWidgetData;
	struct BackgroundBoxWidgetData pressedBackgroundBoxWidgetData;
	struct WidgetPadding textPadding;
	bool bPressed;
	HWidget RootWidget;
	enum ButtonType type;
	int viewmodelRegIndex;
	char onPressFunctionName[MAX_SCRIPT_FUNCTION_NAME_SIZE];
	bool bLuaCallbackSet;
};

static void MakeDefaultTextButtonWidgetData(struct TextButtonWidgetData* pData, struct XMLUIData* pUILayerData)
{
	pData->textPadding.paddingBottom = 20;
	pData->textPadding.paddingTop = 20;
	pData->textPadding.paddingLeft = 20;
	pData->textPadding.paddingRight = 20;
	const char* defaultBackgroundWidget = "defaultButton";
	const char* defaultPressedBackgroundWidget = "defaultPressedButton";

	// background widget

	pData->backgroundBoxWidgetData.atlas = pUILayerData->atlas;
	pData->pressedBackgroundBoxWidgetData.atlas = pUILayerData->atlas;

	pData->backgroundBoxWidgetData.scale.scaleX = 1.0f;
	pData->backgroundBoxWidgetData.scale.scaleY = 1.0f;

	pData->pressedBackgroundBoxWidgetData.scale.scaleX = 1.0f;
	pData->pressedBackgroundBoxWidgetData.scale.scaleY = 1.0f;

	pData->backgroundBoxWidgetData.imageName = malloc(strlen(defaultBackgroundWidget) + 1);
	strcpy(pData->backgroundBoxWidgetData.imageName, defaultBackgroundWidget);
	pData->backgroundBoxWidgetData.sprite = At_FindSprite(pData->backgroundBoxWidgetData.imageName, pData->backgroundBoxWidgetData.atlas);

	pData->pressedBackgroundBoxWidgetData.imageName = malloc(strlen(defaultPressedBackgroundWidget) + 1);
	strcpy(pData->pressedBackgroundBoxWidgetData.imageName, defaultPressedBackgroundWidget);
	pData->pressedBackgroundBoxWidgetData.sprite = At_FindSprite(pData->pressedBackgroundBoxWidgetData.imageName, pData->pressedBackgroundBoxWidgetData.atlas);


	// text widget
	pData->textWidgetData.atlas = pUILayerData->atlas;

	pData->textWidgetData.content = malloc(strlen("Button") + 1);
	strcpy(pData->textWidgetData.content, "Button");
	
	pData->textWidgetData.r = 0.0f;
	pData->textWidgetData.g = 0.0f;
	pData->textWidgetData.b = 0.0f;
	pData->textWidgetData.a = 1.0f;
	
	pData->textWidgetData.fSizePts = 32.0f;
	pData->textWidgetData.font = Fo_FindFont(pData->textWidgetData.atlas, "default", 32.0f);

	pData->bPressed = false;
}

static float GetWidth(struct UIWidget* pWidget, struct UIWidget* pParent)
{
	struct TextButtonWidgetData* pWD = pWidget->pImplementationData;
	struct TextWidgetData* pData = &pWD->textWidgetData;
	return Fo_StringWidth(pData->atlas, pData->font, pData->content) +
		pWD->textPadding.paddingLeft +
		pWD->textPadding.paddingRight +
		pWidget->padding.paddingLeft +
		pWidget->padding.paddingRight;
}

static float GetHeight(struct UIWidget* pWidget, struct UIWidget* pParent)
{
	struct TextButtonWidgetData* pWD = pWidget->pImplementationData;
	struct TextWidgetData* pData = &pWD->textWidgetData;
	return Fo_StringHeight(pData->atlas, pData->font, pData->content) +
		pWD->textPadding.paddingTop +
		pWD->textPadding.paddingBottom +
		pWidget->padding.paddingTop +
		pWidget->padding.paddingBottom;
}

static void LayoutChildren(struct UIWidget* pWidget, struct UIWidget* pParent)
{
	EASSERT(pWidget->hFirstChild == NULL_HWIDGET);
}

static void OnDestroy(struct UIWidget* pWidget)
{
	struct TextButtonWidgetData* pWD = pWidget->pImplementationData;
	TextWidget_Destroy(&pWD->textWidgetData);
	BackgroundBoxWidget_Destroy(&pWD->pressedBackgroundBoxWidgetData);
	BackgroundBoxWidget_Destroy(&pWD->backgroundBoxWidgetData);
	free(pWidget->pImplementationData);
}

static void* OnOutputVerts(struct UIWidget* pWidget, VECTOR(WidgetVertex) pOutVerts)
{
	struct TextButtonWidgetData* pWD = pWidget->pImplementationData;
	struct TextWidgetData* pTextWidgetData = &pWD->textWidgetData;
	struct BackgroundBoxWidgetData* pBBWD = pWD->bPressed ? &pWD->pressedBackgroundBoxWidgetData : &pWD->backgroundBoxWidgetData;
	pOutVerts = BackgroundBoxWidget_OutputVerts(
		pBBWD,
		pOutVerts,
		GetWidth(pWidget, NULL),
		GetHeight(pWidget, NULL),
		&pWidget->padding,
		pWidget->left,
		pWidget->top
	);
	pOutVerts = TextWidget_OutputVerts(pWidget->left + pWidget->padding.paddingLeft, pWidget->top + pWidget->padding.paddingTop, &pWD->textPadding, pTextWidgetData, pOutVerts);
	
	pOutVerts = UI_Helper_OnOutputVerts(pWidget, pOutVerts);
	return pOutVerts;
}


static void MouseButtonDownCallback(struct UIWidget* pWidget, float x, float y, int btn)
{
	struct TextButtonWidgetData* pWD = pWidget->pImplementationData;
	pWD->bPressed = true;
	SetRootWidgetIsDirty(pWD->RootWidget, true);
	if (pWD->bLuaCallbackSet && pWD->type == BT_FireOnDown)
	{
		Sc_CallFuncInRegTableEntryTable(pWD->viewmodelRegIndex, pWD->onPressFunctionName, NULL, 0, 0);
	}
}

static void MouseButtonUpCallback(struct UIWidget* pWidget, float x, float y, int btn)
{
	struct TextButtonWidgetData* pWD = pWidget->pImplementationData;
	pWD->bPressed = false;
	SetRootWidgetIsDirty(pWD->RootWidget, true);
	if (pWD->bLuaCallbackSet && pWD->type == BT_FireOnRelease)
	{
		Sc_CallFuncInRegTableEntryTable(pWD->viewmodelRegIndex, pWD->onPressFunctionName, NULL, 0, 0);
	}
}

static void MouseLeaveCallback(struct UIWidget* pWidget, float x, float y)
{
	struct TextButtonWidgetData* pWD = pWidget->pImplementationData;
	if (pWD->bPressed)
	{
		pWD->bPressed = false;
		SetRootWidgetIsDirty(pWD->RootWidget, true);
	}
}

static void SetCMouseCallbacks(struct UIWidget* pWidget)
{
	pWidget->cCallbacks.Callbacks[WC_OnMouseDown].type = WC_OnMouseDown;
	pWidget->cCallbacks.Callbacks[WC_OnMouseDown].callback.mouseBtnFn = &MouseButtonDownCallback;

	pWidget->cCallbacks.Callbacks[WC_OnMouseUp].type = WC_OnMouseUp;
	pWidget->cCallbacks.Callbacks[WC_OnMouseUp].callback.mouseBtnFn = &MouseButtonUpCallback;

	pWidget->cCallbacks.Callbacks[WC_OnMouseLeave].type = WC_OnMouseLeave;
	pWidget->cCallbacks.Callbacks[WC_OnMouseLeave].callback.mousePosFn = &MouseLeaveCallback;
}

static void MakeWidgetIntoTextButtonWidget(HWidget hWidget, struct DataNode* pDataNode, struct XMLUIData* pUILayerData)
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
	pWidget->pImplementationData = malloc(sizeof(struct TextButtonWidgetData));
	memset(pWidget->pImplementationData, 0, sizeof(struct TextButtonWidgetData));
	struct TextButtonWidgetData* pData = pWidget->pImplementationData;
	pData->RootWidget = pUILayerData->rootWidget;
	pData->viewmodelRegIndex = pUILayerData->hViewModel;
	MakeDefaultTextButtonWidgetData(pData, pUILayerData);
	TextWidget_FromXML(pWidget, &pData->textWidgetData, pDataNode, pUILayerData);
	BackgroundBoxWidget_fromXML(pWidget, &pData->backgroundBoxWidgetData, pDataNode, pUILayerData);
	SetCMouseCallbacks(pWidget);

	if(pDataNode->fnGetPropType(pDataNode, "onPress") == DN_String)
	{
		size_t len = pDataNode->fnGetStrlen(pDataNode, "onPress");
		EASSERT(len < MAX_SCRIPT_FUNCTION_NAME_SIZE);
		pDataNode->fnGetStrcpy(pDataNode, "onPress", pData->onPressFunctionName);
		pData->bLuaCallbackSet = true;
	}
	if(pDataNode->fnGetPropType(pDataNode, "btnType") == DN_String)
	{
		if (pDataNode->fnStrCmp(pDataNode, "btnType", "OnRelease"))
		{
			pData->type = BT_FireOnRelease;
		}
		if (pDataNode->fnStrCmp(pDataNode, "btnType", "OnDown"))
		{
			pData->type = BT_FireOnDown;
		}
	}
}

HWidget TextButtonWidgetNew(HWidget hParent, struct DataNode* pDataNode, struct XMLUIData* pUILayerData)
{
	HWidget hWidget = UI_NewBlankWidget();
	MakeWidgetIntoTextButtonWidget(hWidget, pDataNode, pUILayerData);
	return hWidget;
}
