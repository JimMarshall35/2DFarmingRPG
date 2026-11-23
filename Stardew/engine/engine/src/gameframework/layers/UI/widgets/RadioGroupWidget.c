#include "RadioGroupWidget.h"
#include "Widget.h"
#include "XMLUIGameLayer.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "Atlas.h"
#include "AssertLib.h"
#include "StackPanelWidget.h"
#include "RadioButtonWidget.h"
#include "RootWidget.h"
#include "Scripting.h"
#include "DataNode.h"
#include "Log.h"

struct RadioGroupData
{
	struct StackPanelWidgetData data;
	int nSelectecedChild;
	HWidget rootWidget;
};

static float GetWidth(struct UIWidget* pWidget, struct UIWidget* pParent)
{
	struct StackPanelWidgetData* pData = &((struct RadioGroupData*)pWidget->pImplementationData)->data;
	return StackPanel_GetWidth(pWidget, pParent, pData);
}

static float GetHeight(struct UIWidget* pWidget, struct UIWidget* pParent)
{
	struct StackPanelWidgetData* pData = &((struct RadioGroupData*)pWidget->pImplementationData)->data;
	return StackPanel_GetHeight(pWidget, pParent, pData);
}

static void LayoutChildren(struct UIWidget* pWidget, struct UIWidget* pParent)
{
	struct StackPanelWidgetData* pData = &((struct RadioGroupData*)pWidget->pImplementationData)->data;
	float w = GetWidth(pWidget, pParent);
	float h = GetHeight(pWidget, pParent);
	StackPanel_LayoutChildren(pWidget, pParent, pData, w, h);
}

static void OnDestroy(struct UIWidget* pWidget)
{
	free(pWidget->pImplementationData);
}

static void* OnOutputVerts(struct UIWidget* pWidget, VECTOR(WidgetVertex) pOutVerts)
{
	pOutVerts = UI_Helper_OnOutputVerts(pWidget, pOutVerts);
	return pOutVerts;
}

static void OnWidgetInit(struct UIWidget* pWidget);

static void OnPropertyChanged(struct UIWidget* pThisWidget, struct WidgetPropertyBinding* pBinding)
{
	//selectedChild
	struct RadioGroupData* pData = pThisWidget->pImplementationData;
	if (strcmp(pBinding->boundPropertyName, "selectedChild") == 0)
	{
		char* fnName = UI_MakeBindingGetterFunctionName(pBinding->name);
		Sc_CallFuncInRegTableEntryTable(pThisWidget->scriptCallbacks.viewmodelTable, fnName, NULL, 0, 1);
		free(fnName);
		//pData->fVal = Sc_Float();
		pData->nSelectecedChild = Sc_Int();
		Sc_ResetStack();
		OnWidgetInit(pThisWidget);
	}

}


static void MouseButtonDownCallback(struct UIWidget* pWidget, float x, float y, int btn)
{
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

static void OnWidgetInit(struct UIWidget* pWidget)
{
	struct RadioGroupData* pData = pWidget->pImplementationData;
	int i = 0;
	HWidget child = pWidget->hFirstChild;
	while (child != NULL_HWIDGET)
	{
		RadioButton_SetSelected(child, i == pData->nSelectecedChild);
		struct UIWidget* pChild = UI_GetWidget(child);
		child = pChild->hNext;
		i++;
	}
	SetRootWidgetIsDirty(pData->rootWidget, true);
}

static void ParseBindingEspressionAttribute(char* attribName, char* attribContent, struct UIWidget* pWidget, struct RadioGroupData* pData, struct XMLUIData* pUILayerData)
{
	if (strcmp(attribName, "selectedChild") == 0)
	{
		UI_AddIntPropertyBinding(pWidget, attribName, attribContent, &pData->nSelectecedChild, pUILayerData->hViewModel);
	}
	else
	{
		Log_Warning("invalid property binding: %s", attribContent);
	}
}

static void PopulateRadioGroupDataFromXML(struct UIWidget* pWidget, struct RadioGroupData* pData, struct DataNode* pDataNode, struct XMLUIData* pUILayerData)
{
	if (pDataNode->fnGetPropType(pDataNode, "selectedChild") == DN_String)
	{
		size_t len = pDataNode->fnGetStrlen(pDataNode, "selectedChild");
		if (!len)
		{
			Log_Warning("empty string");
			return;
		}
		char* val = malloc(len + 1);
		pDataNode->fnGetStrcpy(pDataNode, "selectedChild", val);
		if (UI_IsAttributeStringABindingExpression(val))
		{
			ParseBindingEspressionAttribute("selectedChild", val, pWidget, pData, pUILayerData);
		}
		else
		{
			pData->nSelectecedChild = atoi(val);
		}
		free(val);
	}
}

static void MakeWidgetIntoRadioGroupWidget(HWidget hWidget, struct DataNode* pDataNode, struct XMLUIData* pUILayerData)
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
	pWidget->fnOnWidgetInit = &OnWidgetInit;
	pWidget->pImplementationData = malloc(sizeof(struct RadioGroupData));

	pWidget->cCallbacks.Callbacks[WC_OnMouseDown].type = WC_OnMouseDown;
	pWidget->cCallbacks.Callbacks[WC_OnMouseDown].callback.mouseBtnFn = &MouseButtonDownCallback;
	
	pWidget->cCallbacks.Callbacks[WC_OnMouseUp].type = WC_OnMouseUp;
	pWidget->cCallbacks.Callbacks[WC_OnMouseUp].callback.mouseBtnFn = &MouseButtonUpCallback;

	pWidget->cCallbacks.Callbacks[WC_OnMouseLeave].type = WC_OnMouseLeave;
	pWidget->cCallbacks.Callbacks[WC_OnMouseLeave].callback.mousePosFn = &MouseLeaveCallback;

	pWidget->cCallbacks.Callbacks[WC_OnMouseMove].type = WC_OnMouseMove;
	pWidget->cCallbacks.Callbacks[WC_OnMouseMove].callback.mousePosFn = &MouseMoveCallback;

	memset(pWidget->pImplementationData, 0, sizeof(struct RadioGroupData));
	struct RadioGroupData* pData = pWidget->pImplementationData;
	pData->nSelectecedChild = 0;
	pData->rootWidget = pUILayerData->rootWidget;
	StackPanel_PopulateDataFromXML(pDataNode, &pData->data);
	PopulateRadioGroupDataFromXML(pWidget, pData, pDataNode, pUILayerData);
}

HWidget RadioGroupWidgetNew(HWidget hParent, struct DataNode* pDataNode, struct XMLUIData* pUILayerData)
{
	HWidget hWidget = UI_NewBlankWidget();
	MakeWidgetIntoRadioGroupWidget(hWidget, pDataNode, pUILayerData);
	return hWidget;
}

void RadioGroup_ChildSelected(HWidget hRadioGroup, struct UIWidget* pRadioButtonChild)
{
	struct UIWidget* pWidget = UI_GetWidget(hRadioGroup);
	struct RadioGroupData* pData = pWidget->pImplementationData;
	int i = 0;
	HWidget hChild = pWidget->hFirstChild;
	while (hChild != NULL_HWIDGET)
	{
		bool bSelected = false;
		struct UIWidget* pChild = UI_GetWidget(hChild);
		if (UI_GetWidget(hChild) == pRadioButtonChild)
		{
			bSelected = true;
			pData->nSelectecedChild = i;
			struct WidgetPropertyBinding* pBinding = UI_FindBinding(pWidget, "selectedChild");
			if (pBinding)
			{
				char* setterName = UI_MakeBindingSetterFunctionName(pBinding->name);
				struct ScriptCallArgument arg;
				arg.type = SCA_int;
				arg.val.i = pData->nSelectecedChild;
				Sc_CallFuncInRegTableEntryTable(pWidget->scriptCallbacks.viewmodelTable, setterName, &arg, 1, 0);
				free(setterName);
			}

		}

		RadioButton_SetSelected(hChild, bSelected);
		hChild = pChild->hNext;
		i++;
	}
	SetRootWidgetIsDirty(pData->rootWidget, true);
}
