#include "Widget.h"
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "Scripting.h"
#include "AssertLib.h"
#include "DataNode.h"
#include "DrawContext.h"
#include "Log.h"

OBJECT_POOL(struct UIWidget) gWidgetPool = NULL;
HWidget gScratchWidget = NULL_HANDLE; 


#define WIDGET_POOL_BOUNDS_CHECK(handle, rVal) OBJ_POOL_BOUNDS_CHECK(handle, rVal, gWidgetPool)
#define WIDGET_POOL_BOUNDS_CHECK_NO_RETURN(handle) OBJ_POOL_BOUNDS_CHECK_NO_RETURN(handle, gWidgetPool)

static void MakeBlankScriptingCallbacks(struct LuaWidgetCallbacks* pCallbacks)
{
	memset(pCallbacks, 0, sizeof(struct LuaWidgetCallbacks));
}

HWidget UI_NewBlankWidget()
{
	HWidget widget = 0;
	gWidgetPool = GetObjectPoolIndex(gWidgetPool, &widget);
	struct UIWidget* pWidget = &gWidgetPool[widget];
	memset(pWidget, 0, sizeof(struct UIWidget));
	pWidget->hFirstChild = NULL_HWIDGET;
	pWidget->hNext = NULL_HWIDGET;
	pWidget->hPrev = NULL_HWIDGET;
	pWidget->hParent = NULL_HWIDGET;
	pWidget->numBindings = 0;
	pWidget->hThis = widget;
	pWidget->width.type = WD_Auto;
	pWidget->height.type = WD_Auto;
	MakeBlankScriptingCallbacks(&pWidget->scriptCallbacks);
	return widget;
}

size_t UI_CountWidgetChildrenPtr(struct UIWidget* pWidget)
{
	size_t count = 0;
	HWidget h = pWidget->hFirstChild;
	while (h != NULL_HWIDGET)
	{
		struct UIWidget* pChild = UI_GetWidget(h);
		count++;
		h = pChild->hNext;
	}
	return count;
}

size_t UI_CountWidgetChildren(HWidget hWidget)
{
	WIDGET_POOL_BOUNDS_CHECK(hWidget, 0);
	struct UIWidget* pWidget = UI_GetWidget(hWidget);
	return UI_CountWidgetChildrenPtr(pWidget);
}

struct UIWidget* UI_GetWidget(HWidget hWidget)
{
	if (hWidget < 0)
	{
		return NULL;
	}
	return &gWidgetPool[hWidget];
}

struct UIWidget* UI_FirstChild(HWidget hWidget)
{
	struct UIWidget* pWidget = UI_GetWidget(hWidget);
	if(!pWidget)
	{
		return NULL;
	}
	return UI_GetWidget(pWidget->hFirstChild);
}

void UI_AddChild(HWidget hParent, HWidget hChild)
{
	WIDGET_POOL_BOUNDS_CHECK_NO_RETURN(hParent)
	WIDGET_POOL_BOUNDS_CHECK_NO_RETURN(hChild)
	struct UIWidget* pParent = UI_GetWidget(hParent);
	struct UIWidget* pChild = UI_GetWidget(hChild);
	pChild->hParent = hParent;
	if (pParent->hFirstChild == NULL_HWIDGET)
	{
		pParent->hFirstChild = hChild;
		return;
	}
	
	struct UIWidget* pLastChild = UI_GetWidget(pParent->hFirstChild);
	HWidget hLastWidget = hChild;
	while (pLastChild->hNext != NULL_HWIDGET)
	{
		pLastChild = UI_GetWidget(pLastChild->hNext);
		hLastWidget = pLastChild->hNext;
	}
	pLastChild->hNext = hChild;
	pChild->hPrev = hLastWidget;
	pChild->hNext = NULL_HWIDGET;

}

void UI_Init()
{
	gWidgetPool = NEW_OBJECT_POOL(struct UIWidget, 256);
	gWidgetPool = GetObjectPoolIndex(gWidgetPool, &gScratchWidget);
}

HWidget UI_GetScratchWiget()
{
	EASSERT(gScratchWidget != NULL_HANDLE);
	return gScratchWidget;
}

void UI_DestroyWidget(HWidget widget)
{
	WIDGET_POOL_BOUNDS_CHECK_NO_RETURN(widget)
	struct UIWidget* pWidget = UI_GetWidget(widget);
	if (pWidget->hNext != NULL_HWIDGET)
	{
		UI_GetWidget(pWidget->hNext)->hPrev = pWidget->hPrev;
	}
	if (pWidget->hPrev != NULL_HWIDGET)
	{
		UI_GetWidget(pWidget->hPrev)->hNext = pWidget->hNext;
	}
	if (widget >= 0 && widget < ObjectPoolCapacity(gWidgetPool))
	{
		gWidgetPool[widget].fnOnDestroy(&gWidgetPool[widget]);
		HWidget child = gWidgetPool[widget].hFirstChild;
		while (child != NULL_HWIDGET)
		{
			UI_DestroyWidget(child);
			child = gWidgetPool[child].hNext;
		}
		FreeObjectPoolIndex(gWidgetPool, widget);
	}
	else
	{
		Log_Error("DestroyWidget: widget %i out of range", widget);
	}
}

void UI_ParseWidgetDimsAttribute(const char* attributeContent, struct WidgetDim* outWidgetDims)
{
	if (strcmp(attributeContent, "auto") == 0)
	{
		outWidgetDims->type = WD_Auto;
		return;
	}
	if (strcmp(attributeContent, "*") == 0)
	{
		outWidgetDims->type = WD_Stretch;
		return;
	}

	char dimType[16];
	float dimVal = 0.0f;
	int numMatched = sscanf(attributeContent, "%f%s", &dimVal, dimType);
	switch (numMatched)
	{
	case 1:
		outWidgetDims->type = WD_Pixels;
		break;
	case 2:
		if (strcmp(dimType, "px") == 0)
		{
			outWidgetDims->type = WD_Pixels;
		}
		else if (strcmp(dimType, "*") == 0)
		{
			outWidgetDims->type = WD_StretchFraction;
		}
		else if (strcmp(dimType, "%") == 0)
		{
			outWidgetDims->type = WD_Percentage;
		}
		break;
	default:
		Log_Error("invalid widget dim value %s", attributeContent);
		EASSERT(false);
		break;
	}
	outWidgetDims->data = dimVal;
}

void UI_ParseWidgetPaddingAttributes(struct DataNode* pInNode, struct WidgetPadding* outWidgetPadding)
{
	if(pInNode->fnGetPropType(pInNode, "paddingTop") == DN_Float || pInNode->fnGetPropType(pInNode, "paddingTop") == DN_Int)
	{
		outWidgetPadding->paddingTop = pInNode->fnGetFloat(pInNode, "paddingTop");
	}
	if(pInNode->fnGetPropType(pInNode, "paddingBottom") == DN_Float || pInNode->fnGetPropType(pInNode, "paddingBottom") == DN_Int)
	{
		outWidgetPadding->paddingBottom = pInNode->fnGetFloat(pInNode, "paddingBottom");
	}
	if(pInNode->fnGetPropType(pInNode, "paddingLeft") == DN_Float || pInNode->fnGetPropType(pInNode, "paddingLeft") == DN_Int)
	{
		outWidgetPadding->paddingLeft = pInNode->fnGetFloat(pInNode, "paddingLeft");
	}
	if(pInNode->fnGetPropType(pInNode, "paddingRight") == DN_Float || pInNode->fnGetPropType(pInNode, "paddingRight") == DN_Int)
	{
		outWidgetPadding->paddingRight = pInNode->fnGetFloat(pInNode, "paddingRight");
	}
}

void UI_ParseHorizontalAlignementAttribute(const char* contents, enum WidgetHorizontalAlignment* outAlignment)
{
	if (strcmp(contents, "left") == 0)
	{
		*outAlignment = WHA_Left;
	}
	else if (strcmp(contents, "middle") == 0)
	{
		*outAlignment = WHA_Middle;
	}
	else if (strcmp(contents, "right") == 0)
	{
		*outAlignment = WHA_Right;
	}
	else
	{
		*outAlignment = WHA_Middle;
	}
}

void UI_ParseVerticalAlignementAttribute(const char* contents, enum WidgetVerticalAlignment* outAlignment)
{
	if (strcmp(contents, "top") == 0)
	{
		*outAlignment = WVA_Top;
	}
	else if (strcmp(contents, "middle") == 0)
	{
		*outAlignment = WVA_Middle;
	}
	else if (strcmp(contents, "bottom") == 0)
	{
		*outAlignment = WVA_Bottom;
	}
	else
	{
		*outAlignment = WVA_Middle;
	}
}

struct WidgetDim* GetWidgetWidthDim(struct UIWidget* pWidget) { return &pWidget->width; }
struct WidgetDim* GetWidgetHeightDim(struct UIWidget* pWidget) { return &pWidget->height; }

/// <summary>
/// helper that finds all child widgets with width type WD_ParentFraction
/// and returns the total of their "*" values
/// </summary>
/// <param name="pWidgetParent"></param>
/// <returns></returns>
static void GetTotalFractionAmongChildren(struct UIWidget* pWidgetParent, float* outV, WidgetDimGetterFn getter)
{
	*outV = 0.0f;
	HWidget hChild = pWidgetParent->hFirstChild;
	while (hChild != NULL_HWIDGET)
	{
		struct UIWidget* pChild = UI_GetWidget(hChild);
		const struct WidgetDim* dim = getter(pChild);

		EASSERT(dim->type == WD_StretchFraction);
		*outV += dim->data;
		hChild = pChild->hNext;
	}
}

typedef float(*GetWorHFn)(struct UIWidget* pW);

float GetWidgetW(struct UIWidget* pW) { return pW->fnGetWidth(pW, UI_GetWidget(pW->hParent)); }
float GetWidgetH(struct UIWidget* pW) { return pW->fnGetHeight(pW, UI_GetWidget(pW->hParent)); }

static void GetTotalAmountAmongChildren(struct UIWidget* pWidgetParent, float* outV, GetWorHFn getWorH, WidgetDimGetterFn dimgetter)
{
	*outV = 0.0f;
	HWidget hChild = pWidgetParent->hFirstChild;
	int stretchCount = 0;
	while (hChild != NULL_HWIDGET)
	{
		struct UIWidget* pChild = UI_GetWidget(hChild);
		const struct WidgetDim* dim = dimgetter(pChild);
		switch (dim->type)
		{
		case WD_Stretch:
			stretchCount++;
			if (stretchCount > 1)
			{
				EASSERT(false);// only one child can have a width or height of *
			}
			break;
		case WD_Auto:
		case WD_Pixels:
			*outV += getWorH(pChild);
			break;
		default:
			EASSERT(false); // only one child can have a width or height of *
			break;
		}

		*outV += dim->data;
		hChild = pChild->hNext;
	}
}

struct UIWidget* FindResolveableDimensionAncestor(struct UIWidget* pWidgetParent, WidgetDimGetterFn getter)
{
	HWidget hParent = pWidgetParent->hParent;
	while (hParent != NULL_HWIDGET)
	{
		struct UIWidget* pParent = UI_GetWidget(hParent);
		EASSERT(pParent);
		const struct WidgetDim* pDim = getter(pParent);
		if (pDim->type == WD_Percentage || pDim->type == WD_StretchFraction || pDim->type == WD_Pixels)
		{
			return pParent;
		}
		hParent = pParent->hParent;
	}
	EASSERT(false); // root widget must be of of fixed size
	return NULL;
}

float UI_ResolveWidthDimPxls(struct UIWidget* pWidget, const struct WidgetDim* dim)
{
	switch (dim->type)
	{
	case	 WD_Stretch:
	{
		struct UIWidget* pResolvableWidgetParent = FindResolveableDimensionAncestor(pWidget, &GetWidgetWidthDim);
		struct UIWidget* pActualWidgetParent = UI_GetWidget(pWidget->hParent);
		float amount = 0;
		GetTotalAmountAmongChildren(pActualWidgetParent, &amount, &GetWidgetW, &GetWidgetWidthDim);
		float parentW = UI_ResolveWidthDimPxls(pResolvableWidgetParent, &pResolvableWidgetParent->height);
		return parentW - amount;
	}
	case WD_Auto:            
	{
		if(pWidget->hFirstChild != NULL_HANDLE)
		{
			// TODO: This should really return the size of pWidget not its first child...
			struct UIWidget* pChild = UI_GetWidget(pWidget->hThis); // REMEMBER: This used to pass pWidget->hFirstChild to UI_GetWidget!!
			return pChild->fnGetWidth(pChild, pWidget);
		}
	}
	case WD_Pixels:          return dim->data;
	case WD_StretchFraction:
	{
		float wFraction;
		//EASSERT(pWidgetParent->width.type != WD_Auto);
		struct UIWidget*  pResolvableWidgetParent = FindResolveableDimensionAncestor(pWidget, &GetWidgetWidthDim);
		struct UIWidget* pActualWidgetParent = UI_GetWidget(pWidget->hParent);
		GetTotalFractionAmongChildren(pActualWidgetParent, &wFraction, &GetWidgetWidthDim);
		float parentW = UI_ResolveWidthDimPxls(pResolvableWidgetParent, &pResolvableWidgetParent->width);
		return (dim->data / wFraction) * parentW;
	}
	case WD_Percentage:
	{
		struct UIWidget* pWidgetParent = FindResolveableDimensionAncestor(pWidget, &GetWidgetWidthDim);
		float parentW = UI_ResolveWidthDimPxls(pWidgetParent, &pWidgetParent->width);
		return dim->data * parentW;
	}
	default:
		break;
	}
	return dim->data;
}

float UI_ResolveHeightDimPxls(struct UIWidget* pWidget, const struct WidgetDim* dim)
{
	switch (dim->type)
	{
	case	 WD_Stretch:
	{
		struct UIWidget* pResolvableWidgetParent = FindResolveableDimensionAncestor(pWidget, &GetWidgetHeightDim);
		struct UIWidget* pActualWidgetParent = UI_GetWidget(pWidget->hParent);
		float amount = 0;
		GetTotalAmountAmongChildren(pActualWidgetParent, &amount, &GetWidgetH, &GetWidgetHeightDim);
		float parentH = UI_ResolveHeightDimPxls(pResolvableWidgetParent, &pResolvableWidgetParent->height);
		return parentH - amount;
	}
	case WD_Auto:            
	{
		if(pWidget->hFirstChild != NULL_HANDLE)
		{
			struct UIWidget* pChild = UI_GetWidget(pWidget->hThis); // REMEMBER: This used to pass pWidget->hFirstChild to UI_GetWidget!!
			return pChild->fnGetHeight(pChild, pWidget);
		}
	}
	case WD_Pixels:          return dim->data;
	case WD_StretchFraction:
	{
		float wFraction;
		//EASSERT(pWidgetParent->width.type != WD_Auto);
		struct UIWidget*  pResolvableWidgetParent = FindResolveableDimensionAncestor(pWidget, &GetWidgetHeightDim);
		struct UIWidget* pActualWidgetParent = UI_GetWidget(pWidget->hParent);
		GetTotalFractionAmongChildren(pActualWidgetParent, &wFraction, &GetWidgetHeightDim);
		float parentW = UI_ResolveHeightDimPxls(pResolvableWidgetParent, &pResolvableWidgetParent->height);
		return (dim->data / wFraction) * parentW;
	}
	case WD_Percentage:
	{
		struct UIWidget* pWidgetParent = FindResolveableDimensionAncestor(pWidget, &GetWidgetHeightDim);
		float parentW = UI_ResolveHeightDimPxls(pWidgetParent, &pWidgetParent->height);
		return dim->data * parentW;
	}
	default:
		break;
	}
	return dim->data;
}


bool UI_ParseWidgetDockPoint(struct DataNode* pInNode, struct UIWidget* outWidget)
{
	if(pInNode->fnGetPropType(pInNode, "dockPoint") == DN_String)
	{
		if (pInNode->fnStrCmp(pInNode, "dockPoint", "topLeft"))
		{
			outWidget->dockPoint = WDP_TopLeft;
			return true;
		}
		else if (pInNode->fnStrCmp(pInNode, "dockPoint", "topMiddle"))
		{
			outWidget->dockPoint = WDP_TopMiddle;
			return true;
		}
		else if (pInNode->fnStrCmp(pInNode, "dockPoint", "topRight"))
		{
			outWidget->dockPoint = WDP_TopRight;
			return true;
		}
		else if (pInNode->fnStrCmp(pInNode, "dockPoint", "middleRight"))
		{
			outWidget->dockPoint = WDP_MiddleRight;
			return true;
		}
		else if (pInNode->fnStrCmp(pInNode, "dockPoint", "bottomRight"))
		{
			outWidget->dockPoint = WDP_BottomRight;
			return true;
		}
		else if (pInNode->fnStrCmp(pInNode, "dockPoint", "bottomMiddle"))
		{
			outWidget->dockPoint = WDP_BottomMiddle;
			return true;
		}
		else if (pInNode->fnStrCmp(pInNode, "dockPoint", "bottomLeft"))
		{
			outWidget->dockPoint = WDP_BottomLeft;
			return true;
		}
		else if (pInNode->fnStrCmp(pInNode, "dockPoint", "middleLeft"))
		{
			outWidget->dockPoint = WDP_MiddleLeft;
			return true;
		}
		else if (pInNode->fnStrCmp(pInNode, "dockPoint", "centre"))
		{
			outWidget->dockPoint = WDP_Centre;
			return true;
		}
	}
	return false;
}

static void ParseLuaCallbacks(struct DataNode* pInNode, struct UIWidget* outWidget)
{
	if(pInNode->fnGetPropType(pInNode, "onMouseEnter") == DN_String)
	{
		pInNode->fnGetStrcpy(pInNode, "onMouseEnter", outWidget->scriptCallbacks.Callbacks[WC_OnMouseEnter].name);
		outWidget->scriptCallbacks.Callbacks[WC_OnMouseEnter].bActive = true;
	}
	if(pInNode->fnGetPropType(pInNode, "onMouseLeave") == DN_String)
	{
		pInNode->fnGetStrcpy(pInNode, "onMouseLeave", outWidget->scriptCallbacks.Callbacks[WC_OnMouseLeave].name);
		outWidget->scriptCallbacks.Callbacks[WC_OnMouseLeave].bActive = true;
	}
	if(pInNode->fnGetPropType(pInNode, "onMouseMove") == DN_String)
	{
		pInNode->fnGetStrcpy(pInNode, "onMouseMove", outWidget->scriptCallbacks.Callbacks[WC_OnMouseMove].name);
		outWidget->scriptCallbacks.Callbacks[WC_OnMouseMove].bActive = true;
	}
	if(pInNode->fnGetPropType(pInNode, "onMouseDown") == DN_String)
	{
		pInNode->fnGetStrcpy(pInNode, "onMouseDown", outWidget->scriptCallbacks.Callbacks[WC_OnMouseDown].name);
		outWidget->scriptCallbacks.Callbacks[WC_OnMouseDown].bActive = true;
	}
	if(pInNode->fnGetPropType(pInNode, "onMouseUp") == DN_String)
	{
		pInNode->fnGetStrcpy(pInNode, "onMouseUp", outWidget->scriptCallbacks.Callbacks[WC_OnMouseUp].name);
		outWidget->scriptCallbacks.Callbacks[WC_OnMouseUp].bActive = true;
	}
}

static void ParseWidgetAlignments(struct DataNode* pInNode, struct UIWidget* outWidget)
{
	outWidget->horizontalAlignment = WHA_Middle;
	outWidget->verticalAlignment = WVA_Middle;

	if(pInNode->fnGetPropType(pInNode, "horizontalAlignment") == DN_String)
	{
		char buffer[64];
		pInNode->fnGetStrcpy(pInNode, "horizontalAlignment", buffer);
		UI_ParseHorizontalAlignementAttribute(buffer, &outWidget->horizontalAlignment);
	}
	if(pInNode->fnGetPropType(pInNode, "verticalAlignment") == DN_String)
	{
		char buffer[64];
		pInNode->fnGetStrcpy(pInNode, "verticalAlignment", buffer);
		UI_ParseVerticalAlignementAttribute(buffer, &outWidget->verticalAlignment);
	}
}

static void ParseWidgetDims(struct DataNode* pInNode, struct UIWidget* outWidget)
{
	if(pInNode->fnGetPropType(pInNode, "width") == DN_String)
	{
		char buffer[64];
		pInNode->fnGetStrcpy(pInNode, "width", buffer);
		UI_ParseWidgetDimsAttribute(buffer, &outWidget->width);
	}
	if(pInNode->fnGetPropType(pInNode, "height") == DN_String)
	{
		char buffer[64];
		pInNode->fnGetStrcpy(pInNode, "height", buffer);
		UI_ParseWidgetDimsAttribute(buffer, &outWidget->height);
	}
}

static void ParseWidgetOffsets(struct DataNode* pInNode, struct UIWidget* outWidget)
{
	if(pInNode->fnGetPropType(pInNode, "x") == DN_Float)
	{
		outWidget->offsetX = pInNode->fnGetFloat(pInNode, "x");
	}
	if(pInNode->fnGetPropType(pInNode, "y") == DN_Float)
	{
		outWidget->offsetY = pInNode->fnGetFloat(pInNode, "y");
	}
}

void UI_DefaultOnChildrenChanged(struct UIWidget* outWidget)
{
	if (outWidget->hParent)
	{
		struct UIWidget* pWidget = UI_GetWidget(outWidget->hParent);
		if (pWidget->fnOnWidgetChildrenChangedFn)
		{
			pWidget->fnOnWidgetChildrenChangedFn(pWidget);
		}
	}
}

void UI_WidgetCommonInit(struct DataNode* pInNode, struct UIWidget* outWidget)
{
	if(!outWidget->fnOnWidgetChildrenChangedFn)
		outWidget->fnOnWidgetChildrenChangedFn = UI_DefaultOnChildrenChanged;
	UI_ParseWidgetPaddingAttributes(pInNode, &outWidget->padding);
	UI_ParseWidgetDockPoint(pInNode, outWidget);
	ParseLuaCallbacks(pInNode, outWidget);
	ParseWidgetDims(pInNode, outWidget);
	ParseWidgetOffsets(pInNode, outWidget);
	ParseWidgetAlignments(pInNode, outWidget);

	if(pInNode->fnGetPropType(pInNode, "childrenBinding") == DN_String)
	{
		struct WidgetPropertyBinding* pBinding = &outWidget->bindings[outWidget->numBindings++];
		pBinding->type = WBT_WidgetTree;
		pInNode->fnGetStrcpy(pInNode, "childrenBinding", pBinding->name);
		strcpy(pBinding->boundPropertyName, "childrenBinding");
	}
}



static const char* GetDockingPointName(WidgetDockPoint dockingPoint)
{
	switch (dockingPoint)
	{
	case WDP_TopLeft: return "TopLeft";
	case WDP_TopMiddle: return "TopMiddle";
	case WDP_TopRight: return "TopRight";
	case WDP_MiddleRight: return "MiddleRight";
	case WDP_BottomRight: return "BottomRight";
	case WDP_BottomMiddle:  return "BottomMiddle";
	case WDP_BottomLeft:  return "BottomLeft";
	case WDP_MiddleLeft:  return "MiddleLeft";
	default: return "Unknown";
	}
}

void UI_DebugPrintCommonWidgetInfo(const struct UIWidget* inWidget, PrintfFn pPrintfFn)
{
	pPrintfFn("paddingTop = %.1f, paddingBottom = %.1f, paddingLeft = %.1f, paddingRight = %.1f, left = %.1f, top = %.1f, dockPoint = %s",
		inWidget->padding.paddingTop,
		inWidget->padding.paddingBottom,
		inWidget->padding.paddingLeft,
		inWidget->padding.paddingRight,
		inWidget->left,
		inWidget->top,
		GetDockingPointName(inWidget->dockPoint));
}

void* UI_Helper_OnOutputVerts(struct UIWidget* pWidget, VECTOR(WidgetVertex) pOutVerts)
{
	HWidget child = pWidget->hFirstChild;
	while (child != NULL_HWIDGET)
	{
		struct UIWidget* pChildWidget = UI_GetWidget(child);
		if (pChildWidget->fnOutputVertices)
		{
			pOutVerts = pChildWidget->fnOutputVertices(pChildWidget, pOutVerts);
		}
		child = pChildWidget->hNext;
	}
	return pOutVerts;
}

void UI_Helper_OnLayoutChildren(struct UIWidget* pWidget, struct UIWidget* pParent)
{
	if (pWidget->hFirstChild == NULL_HWIDGET)
	{
		return;
	}
	struct UIWidget* pChild = UI_GetWidget(pWidget->hFirstChild);
	while (pChild)
	{
		pChild->fnLayoutChildren(pChild, pWidget);
		pChild = UI_GetWidget(pChild->hNext);
	}
}

void UI_SendWidgetMouseEvent(struct UIWidget* pWidget, enum WidgetCallbackTypes type, struct WidgetMouseInfo* pMouseInfo)
{
	if (pWidget->cCallbacks.Callbacks[type].callback.bActive)
	{
		switch (type)
		{
		case WC_OnMouseLeave:
		case WC_OnMouseEnter:
		case WC_OnMouseMove:
			pWidget->cCallbacks.Callbacks[type].callback.mousePosFn(pWidget, pMouseInfo->x, pMouseInfo->y);
			break;
		case WC_OnMouseDown:
		case WC_OnMouseUp:
			pWidget->cCallbacks.Callbacks[type].callback.mouseBtnFn(pWidget, pMouseInfo->x, pMouseInfo->y, pMouseInfo->button);
			break;
		
		}
	}
	if (!pWidget->scriptCallbacks.Callbacks[type].bActive)
	{
		return;
	}
	int numArguments = 0;
	struct ScriptCallArgument arguments[8];

	switch (type)
	{
	case WC_OnMouseLeave:
	case WC_OnMouseEnter:
	case WC_OnMouseMove:
	{
		struct ScriptCallArgument* pX = &arguments[numArguments++];
		struct ScriptCallArgument* pY = &arguments[numArguments++];
		pX->type = SCA_number;
		pX->val.number = pMouseInfo->x;
		pY->type = SCA_number;
		pY->val.number = pMouseInfo->y;
	}
		break;
	case WC_OnMouseDown:
	case WC_OnMouseUp:
	{
		struct ScriptCallArgument* pX = &arguments[numArguments++];
		struct ScriptCallArgument* pY = &arguments[numArguments++];
		struct ScriptCallArgument* pMouseButton = &arguments[numArguments++];

		pX->type = SCA_number;
		pX->val.number = pMouseInfo->x;
		pY->type = SCA_number;
		pY->val.number = pMouseInfo->y;
		pMouseButton->type = SCA_number;
		pMouseButton->val.number = pMouseInfo->button;
	}
		break;
	}

	Sc_CallFuncInRegTableEntryTable(
		pWidget->scriptCallbacks.viewmodelTable,
		pWidget->scriptCallbacks.Callbacks[type].name,
		&arguments[0], 
		numArguments,
		0
	);
}

void UI_GetWidgetSize(HWidget hWidget, float* pOutW, float* pOutH)
{
	WIDGET_POOL_BOUNDS_CHECK_NO_RETURN(hWidget);

	struct UIWidget* pWidget = UI_GetWidget(hWidget);
	*pOutW = pWidget->fnGetWidth(pWidget, NULL);
	*pOutH = pWidget->fnGetHeight(pWidget, NULL);
}

void UI_GetWidgetPadding(HWidget hWidget, float* pOutPaddingTop, float* pOutPaddingBottom, float* pOutPaddingLeft, float* pOutPaddingRight)
{
	WIDGET_POOL_BOUNDS_CHECK_NO_RETURN(hWidget);

	struct UIWidget* pWidget = UI_GetWidget(hWidget);
	*pOutPaddingTop = pWidget->padding.paddingTop;
	*pOutPaddingBottom = pWidget->padding.paddingBottom;
	*pOutPaddingLeft = pWidget->padding.paddingLeft;
	*pOutPaddingRight = pWidget->padding.paddingRight;
}

void UI_GetWidgetTopLeft(HWidget hWidget, float* pOutLeft, float* pOutTop)
{
	WIDGET_POOL_BOUNDS_CHECK_NO_RETURN(hWidget);
	struct UIWidget* pWidget = UI_GetWidget(hWidget);
	*pOutLeft = pWidget->left;
	*pOutTop = pWidget->top;
}	

void UI_GetHitBox(GeomRect outRect, HWidget hWidget)
{
	float padTop, padBot, padLeft, padRight;
	float top, left;
	float widgth, height;
	UI_GetWidgetPadding(hWidget, &padTop, &padBot, &padLeft, &padRight);
	UI_GetWidgetTopLeft(hWidget, &left, &top);
	UI_GetWidgetSize(hWidget, &widgth, &height);
	widgth -= padLeft + padRight;
	height -= padTop + padBot;
	outRect[GR_TLX] = left + padLeft;
	outRect[GR_TLY] = top + padTop;
	outRect[GR_BRX] = outRect[GR_TLX] + widgth;
	outRect[GR_BRY] = outRect[GR_TLY] + height;
}

bool UI_IsAttributeStringABindingExpression(const char* attributeValue)
{
	return attributeValue[0] == '{' && attributeValue[strlen(attributeValue) - 1] == '}';
}


char* UI_MakeBindingGetterFunctionName(const char* inBindingName)
{
	char* fnName = malloc(strlen(inBindingName) + 1 + 4);
	char* writePtr = fnName;
	*writePtr++ = 'G';
	*writePtr++ = 'e';
	*writePtr++ = 't';
	*writePtr++ = '_';
	strcpy(writePtr, inBindingName);
	return fnName;
}

char* UI_MakeBindingSetterFunctionName(const char* inBindingName)
{
	char* fnName = malloc(strlen(inBindingName) + 1 + 4);
	char* writePtr = fnName;
	*writePtr++ = 'S';
	*writePtr++ = 'e';
	*writePtr++ = 't';
	*writePtr++ = '_';
	strcpy(writePtr, inBindingName);
	return fnName;
}

struct WidgetPropertyBinding* UI_FindBinding(struct UIWidget* pWidget, const char* bindingName)
{
	for (int i = 0; i < pWidget->numBindings; i++)
	{
		if (strcmp(pWidget->bindings[i].boundPropertyName, bindingName) == 0)
		{
			return &pWidget->bindings[i];
		}
	}
	return NULL;
}


static char* PopulateBinding(struct WidgetPropertyBinding* pBinding, char* inBindingExpression, char* inBoundPropertyName)
{
	inBindingExpression[strlen(inBindingExpression) - 1] = '\0';
	inBindingExpression++;
	strcpy(pBinding->name, inBindingExpression);
	strcpy(pBinding->boundPropertyName, inBoundPropertyName);
	return inBindingExpression;
}

void UI_AddStringPropertyBinding(struct UIWidget* pWidget, char* inBoundPropertyName, char* inBindingExpression, char** pOutData, int viewmodelTableIndex)
{
	if (pWidget->numBindings >= MAX_NUM_BINDINGS)
	{
		printf("MAX_NUM_BINDINGS exceeded\n");
		EASSERT(false);
		return;
	}
	struct WidgetPropertyBinding* pBinding = &pWidget->bindings[pWidget->numBindings++];
	inBindingExpression = PopulateBinding(pBinding, inBindingExpression, inBoundPropertyName);

	pBinding->type = WBT_String;
	pBinding->data.str = NULL;
	char* fnName = UI_MakeBindingGetterFunctionName(inBindingExpression);

	Sc_CallFuncInRegTableEntryTable(viewmodelTableIndex, fnName, NULL, 0, 1);
	size_t strLen =  Sc_StackTopStringLen();
	*pOutData = malloc(strLen + 1);
	Sc_StackTopStrCopy(*pOutData);
	Sc_ResetStack();
	free(fnName);
}

void UI_AddIntPropertyBinding(struct UIWidget* pWidget, char* inBoundPropertyName, char* inBindingExpression, int* pOutData, int viewmodelTableIndex)
{
	if (pWidget->numBindings >= MAX_NUM_BINDINGS)
	{
		Log_Error("MAX_NUM_BINDINGS exceeded");
		EASSERT(false);
		return;
	}
	struct WidgetPropertyBinding* pBinding = &pWidget->bindings[pWidget->numBindings++];
	inBindingExpression = PopulateBinding(pBinding, inBindingExpression, inBoundPropertyName);

	pBinding->type = WBT_Int;
	pBinding->data.str = NULL;
	char* fnName = UI_MakeBindingGetterFunctionName(inBindingExpression);

	Sc_CallFuncInRegTableEntryTable(viewmodelTableIndex, fnName, NULL, 0, 1);
	int i = Sc_Int();
	Sc_ResetStack();
	*pOutData = i;
}

void UI_AddFloatPropertyBinding(struct UIWidget* pWidget, char* inBoundPropertyName, char* inBindingExpression, float* pOutData, int viewmodelTableIndex)
{
	if (pWidget->numBindings >= MAX_NUM_BINDINGS)
	{
		Log_Error("MAX_NUM_BINDINGS exceeded");
		EASSERT(false);
		return;
	}
	struct WidgetPropertyBinding* pBinding = &pWidget->bindings[pWidget->numBindings++];
	inBindingExpression = PopulateBinding(pBinding, inBindingExpression, inBoundPropertyName);

	pBinding->type = WBT_Float;
	pBinding->data.str = NULL;
	char* fnName = UI_MakeBindingGetterFunctionName(inBindingExpression);

	Sc_CallFuncInRegTableEntryTable(viewmodelTableIndex, fnName, NULL, 0, 1);
	float i = Sc_Float();
	Sc_ResetStack();
	*pOutData = i;

}
