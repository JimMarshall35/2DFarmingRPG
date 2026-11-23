#include "BarWidget.h"
#include "Atlas.h"
#include "DataNode.h"
#include "XMLUIGameLayer.h"
#include "WidgetVertexOutputHelpers.h"
#include "AssertLib.h"
#include <string.h>
#include "Log.h"
#include "Widget.h"
#include "Scripting.h"
#include "RootWidget.h"

void* BarWidget_OnOutputVerts(
    VECTOR(WidgetVertex) pOutVerts, 
    struct BarData* pData, 
    float top, 
    float left, 
    struct WidgetPadding* pPadding
)
{
	WidgetQuad quads[6];
	top = top + pPadding->paddingTop;
	left = left + pPadding->paddingLeft;
	float f = fabs(pData->fVal);
	f = f > 1.0f ? 1.0f : f;

    /* frame */
    ThreePanelScale(
        left, top,
        quads,
        pData->orientation == BO_Horizontal ? WI_Horizontal : WI_Vertical,
        pData->lengthPx,
        pData->frameStaticData.sprite,
        pData->frameStaticData.atlas
    );

    /* fill */
    ThreePanelScale(
        left, top,
        quads + 3,
        pData->orientation == BO_Horizontal ? WI_Horizontal : WI_Vertical,
        pData->lengthPx * f,
        pData->fillStaticData.sprite,
        pData->fillStaticData.atlas
    );

    vec2 adjustment = { 0.0f, 0.0f };

    switch(pData->orientation)
    {
    case BO_Horizontal:
        {
            switch (pData->fillAnchorPt)
            {
            case AP_RightOrBottom:
                {
                    adjustment[0] = pData->lengthPx - (pData->lengthPx * pData->fVal);
                }
                break;
            case AP_Middle:
                {
                    adjustment[0] = (pData->lengthPx - (pData->lengthPx * pData->fVal)) / 2.0f;
                }
                break;
            }
        }
        break;
    case BO_Vertical:
        {
            switch (pData->fillAnchorPt)
            {
            case AP_RightOrBottom:
                {
                    adjustment[1] = pData->lengthPx - (pData->lengthPx * pData->fVal);
                }
                break;
            case AP_Middle:
                {
                    adjustment[1] = (pData->lengthPx - (pData->lengthPx * pData->fVal)) / 2.0f;
                }
                break;
            }
        }
        break;
    }
    if(pData->fillAnchorPt != AP_TopOrLeft)
    {
        for(int i=3; i<6; i++)
        {
            TranslateWidgetQuad(adjustment, &quads[i]);
        }
    }
    
    return OutputWidgetQuads(pOutVerts, quads, 6);
}

float BarWidget_GetWidth(struct BarData* pData, struct WidgetPadding* pPadding)
{
	switch (pData->orientation)
	{
	case BO_Horizontal:
		return pData->lengthPx + pPadding->paddingLeft + pPadding->paddingRight;
	case BO_Vertical:
	{
		AtlasSprite* pSprite = At_GetSprite(pData->frameStaticData.sprite, pData->frameStaticData.atlas);
		return pSprite->widthPx * pData->frameStaticData.scale.scaleX + pPadding->paddingLeft + pPadding->paddingRight;
	}
	default:
		EASSERT(false);
		break;
	}
	return 0.0f;
}

float BarWidget_GetHeight(struct BarData* pData, struct WidgetPadding* pPadding)
{
	switch (pData->orientation)
	{
	case BO_Horizontal:
	{
		AtlasSprite* pSprite = At_GetSprite(pData->frameStaticData.sprite, pData->frameStaticData.atlas);
		return pSprite->heightPx * pData->frameStaticData.scale.scaleY + pPadding->paddingTop + pPadding->paddingBottom;
	}
	case BO_Vertical:
	{
		return pData->lengthPx + pPadding->paddingTop + pPadding->paddingBottom;
	}
	default:
		EASSERT(false);
		break;
	}

	return 0.0f;
}

static void ParseBindingEspressionAttribute(char* attribName, char* attribContent, struct UIWidget* pWidget, struct BarData* pData, struct XMLUIData* pUILayerData)
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


static void MakeBarFromXML(struct UIWidget* pWidget, struct BarData* pData, struct DataNode* pXMLNode, struct XMLUIData* pUILayerData)
{
	PopulateStaticInternal(pXMLNode, &pData->frameStaticData, pUILayerData,
		"frameSprite",
		"frameScaleX",
		"frameScaleY");
	PopulateStaticInternal(pXMLNode, &pData->fillStaticData, pUILayerData,
		"fillSprite",
		"fillScaleX",
		"fillScaleY");
	if(pXMLNode->fnGetPropType(pXMLNode, "orientation") == DN_String)
	{
		if(pXMLNode->fnStrCmp(pXMLNode, "orientation", "horizontal"))
		{
			pData->orientation = BO_Horizontal;
		}
		else if (pXMLNode->fnStrCmp(pXMLNode, "orientation", "vertical"))
		{
			pData->orientation = BO_Vertical;
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
        else
        {
            Log_Error("bar widget: invalid property binding: %s", attribute);
        }
		free(attribute);
	}
    if(pXMLNode->fnGetPropType(pXMLNode, "lengthPx") == DN_Float)
    {
        pData->lengthPx = pXMLNode->fnGetFloat(pXMLNode, "lengthPx");
    }
    if(pXMLNode->fnGetPropType(pXMLNode, "fillAnchorPoint") == DN_String)
    {
        if(pXMLNode->fnStrCmp(pXMLNode, "fillAnchorPoint", "topOrLeft"))
		{
			pData->fillAnchorPt = AP_TopOrLeft;
		}
        else if(pXMLNode->fnStrCmp(pXMLNode, "fillAnchorPoint", "middle"))
		{
			pData->fillAnchorPt = AP_Middle;
		}
        else if(pXMLNode->fnStrCmp(pXMLNode, "fillAnchorPoint", "rightOrBottom"))
		{
			pData->fillAnchorPt = AP_RightOrBottom;
		}
        else
        {
            Log_Error("MakeBarFromXML, invalid fillAnchorPt value.");
        }
    }
}

static float GetWidth(struct UIWidget* pWidget, struct UIWidget* pParent)
{
	struct BarData* pData = pWidget->pImplementationData;
	return BarWidget_GetWidth(pData, &pWidget->padding);
}

static float GetHeight(struct UIWidget* pWidget, struct UIWidget* pParent)
{
	struct BarData* pData = pWidget->pImplementationData;
	return BarWidget_GetHeight(pData, &pWidget->padding);

}

static void LayoutChildren(struct UIWidget* pWidget, struct UIWidget* pParent)
{
}

static void OnDestroy(struct UIWidget* pWidget)
{
	free(pWidget->pImplementationData);
}

static void* OnOutputVerts(struct UIWidget* pWidget, VECTOR(WidgetVertex) pOutVerts)
{
	struct BarData* pData = pWidget->pImplementationData;
	return BarWidget_OnOutputVerts(pOutVerts, pData, pWidget->top, pWidget->left, &pWidget->padding);
}

static void OnPropertyChanged(struct UIWidget* pThisWidget, struct WidgetPropertyBinding* pBinding)
{
	struct BarData* pData = pThisWidget->pImplementationData;
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



static void MakeWidgetIntoBarWidget(HWidget hWidget, struct DataNode* pDataNode, struct XMLUIData* pUILayerData)
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
	pWidget->pImplementationData = malloc(sizeof(struct BarData));
    

	// pWidget->cCallbacks.Callbacks[WC_OnMouseDown].type = WC_OnMouseDown;
	// pWidget->cCallbacks.Callbacks[WC_OnMouseDown].callback.mouseBtnFn = &MouseButtonDownCallback;
	
	// pWidget->cCallbacks.Callbacks[WC_OnMouseUp].type = WC_OnMouseUp;
	// pWidget->cCallbacks.Callbacks[WC_OnMouseUp].callback.mouseBtnFn = &MouseButtonUpCallback;

	// pWidget->cCallbacks.Callbacks[WC_OnMouseLeave].type = WC_OnMouseLeave;
	// pWidget->cCallbacks.Callbacks[WC_OnMouseLeave].callback.mousePosFn = &MouseLeaveCallback;

	// pWidget->cCallbacks.Callbacks[WC_OnMouseMove].type = WC_OnMouseMove;
	// pWidget->cCallbacks.Callbacks[WC_OnMouseMove].callback.mousePosFn = &MouseMoveCallback;

	memset(pWidget->pImplementationData, 0, sizeof(struct BarData));

    //SliderWidget_MakeDefaultSliderWidget(pWidget->pImplementationData, pUILayerData, SO_Horizontal);
	struct BarData* pData = pWidget->pImplementationData;
	pData->rootWidget = pUILayerData->rootWidget;
	pData->fVal = 1.0f;
    pData->orientation = BO_Horizontal;
    pData->fillAnchorPt = AP_TopOrLeft;

	MakeBarFromXML(pWidget, pData, pDataNode, pUILayerData);
}

HWidget BarWidgetNew(HWidget hParent, struct DataNode* pXMLNode, struct XMLUIData* pUILayerData)
{
	HWidget hWidget = UI_NewBlankWidget();
	MakeWidgetIntoBarWidget(hWidget, pXMLNode, pUILayerData);
	return hWidget;
}
