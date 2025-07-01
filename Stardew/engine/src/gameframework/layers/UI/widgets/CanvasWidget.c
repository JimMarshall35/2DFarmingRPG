#include "CanvasWidget.h"
#include "Widget.h"
#include <libxml/tree.h>
#include "XMLUIGameLayer.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "Atlas.h"
#include "AssertLib.h"
#include "RootWidget.h"
#include "Geometry.h"
#include "WidgetVertexOutputHelpers.h"

static struct WidgetPadding zeroPadding =
{
	0,0,0,0
};



static float BBWidth(GeomRect r)
{
	return r[2] - r[0];

}

static float BBHeight(GeomRect r)
{
	return r[3] - r[1];
}


static float GetWidth(struct UIWidget* pWidget, struct UIWidget* pParent)
{
	return UI_ResolveWidthDimPxls(pWidget, &pWidget->width);
}

static float GetHeight(struct UIWidget* pWidget, struct UIWidget* pParent)
{
	return UI_ResolveHeightDimPxls(pWidget, &pWidget->height);
}

/// <summary>
/// STORES RESULT IN tl1, br1
/// </summary>
static void UnionRects(vec2 tl1, vec2 br1, vec2 tl2, vec2 br2)
{
	if (tl2[0] < tl1[0])
	{
		tl1[0] = tl2[0];
	}
	if (tl2[1] < tl1[1])
	{
		tl1[1] = tl2[1];
	}
	if (br2[0] > br1[0])
	{
		br1[0] = br2[0];
	}
	if (br2[1] > br1[1])
	{
		br1[1] = br2[1];
	}
}

static void GetChildrenBoundingBox(struct UIWidget* pWidget, vec2 tl, vec2 br)
{
	bool bSet = false;
	HWidget hChild = pWidget->hFirstChild;
	while (hChild != NULL_HWIDGET)
	{

		vec2 tl0, br0;
		struct UIWidget* pChild = UI_GetWidget(hChild);
		tl0[0] = pChild->left;
		tl0[1] = pChild->top;
		vec2 dims;
		UI_GetWidgetSize(hChild, &dims[0], &dims[1]);
		glm_vec2_add(tl0, dims, br0);
		if (!bSet)
		{
			memcpy(tl, tl0, sizeof(vec2));
			memcpy(br, br0, sizeof(vec2));
			bSet = true;
			hChild = pChild->hNext;
			continue; // nothing to see here
		}
		UnionRects(tl, br, tl0, br0);
	}
}

static void SetScroll(struct CanvasData* pData)
{
	if (pData->bVSliderActive)
	{
		pData->scrollY = pData->sliderV.fVal;
	}
	else
	{
		pData->scrollY = 0;
	}
	if (pData->bHSliderActive)
	{
		pData->scrollX = pData->sliderH.fVal;
	}
	else
	{
		pData->scrollX = 0;
	}
}

static void LayoutChildren(struct UIWidget* pWidget, struct UIWidget* pParent)
{
	struct CanvasData* pData = pWidget->pImplementationData;
	SetScroll(pData);
	RootWidget_LayoutChildren(pWidget, pParent, pData->scrollX, pData->scrollY);
	GetChildrenBoundingBox(pWidget, pData->contentBB, &pData->contentBB[2]);
}

static void OnDestroy(struct UIWidget* pWidget)
{
	free(pWidget->pImplementationData);
}

static bool ContentExceedsSize(struct CanvasData* pCanvasData, struct UIWidget* pWidget, bool* pOutExceedsWidth, bool* pOutExceedsHeight)
{
	float w = pWidget->fnGetWidth(pWidget, UI_GetWidget(pWidget->hParent)) - (pWidget->padding.paddingRight + pWidget->padding.paddingLeft);
	float h = pWidget->fnGetHeight(pWidget, UI_GetWidget(pWidget->hParent)) - (pWidget->padding.paddingTop + pWidget->padding.paddingBottom);
	
	float contentW = BBWidth(pCanvasData->contentBB);
	float contentH = BBHeight(pCanvasData->contentBB);
	*pOutExceedsWidth = contentW > w;
	*pOutExceedsHeight = contentH > h;

	return *pOutExceedsWidth || *pOutExceedsHeight;
}

void GetClipRegion(GeomRect rect, struct UIWidget* pWidget)
{
	float w = pWidget->fnGetWidth(pWidget, UI_GetWidget(pWidget->hParent)) - (pWidget->padding.paddingRight + pWidget->padding.paddingLeft);
	float h = pWidget->fnGetHeight(pWidget, UI_GetWidget(pWidget->hParent)) - (pWidget->padding.paddingTop + pWidget->padding.paddingBottom);
	rect[0] = pWidget->left + pWidget->padding.paddingLeft;
	rect[1] = pWidget->top + pWidget->padding.paddingTop;
	rect[2] = rect[0] + w;
	rect[3] = rect[1] + h;
}

static void SetSliderPositionAndDims(struct UIWidget* pWidget, struct CanvasData* pData)
{
	float canvasH = pWidget->fnGetHeight(pWidget, UI_GetWidget(pWidget->hParent));
	float canvasW = pWidget->fnGetWidth(pWidget, UI_GetWidget(pWidget->hParent));
	pData->sliderV.lengthPx = canvasH;
	pData->sliderH.lengthPx = canvasW;
	vec2 canvasTL = {
		pWidget->left,
		pWidget->top
	};
	pData->sliderHTopLeft[0] = canvasTL[0];
	pData->sliderHTopLeft[1] = canvasTL[1] + (canvasH - SliderWidget_GetHeight(&pData->sliderH, &zeroPadding));

	pData->sliderVTopLeft[0] = canvasTL[0] + (canvasW - SliderWidget_GetWidth(&pData->sliderV, &zeroPadding));
	pData->sliderVTopLeft[1] = canvasTL[1];
}

static void SetSliderMinAndMax(struct UIWidget* pWidget, struct CanvasData* pCanvasData)
{
	float w = pWidget->fnGetWidth(pWidget, UI_GetWidget(pWidget->hParent)) - (pWidget->padding.paddingRight + pWidget->padding.paddingLeft);
	float h = pWidget->fnGetHeight(pWidget, UI_GetWidget(pWidget->hParent)) - (pWidget->padding.paddingTop + pWidget->padding.paddingBottom);
	
	float left = pWidget->left + pWidget->padding.paddingLeft;
	float top = pWidget->top + pWidget->padding.paddingTop;
	float amountAbove = top - pCanvasData->contentBB[1];
	float amountBelow = pCanvasData->contentBB[3] - (top + h);
	float amoutLeft = left - pCanvasData->contentBB[0];
	float amountRight = pCanvasData->contentBB[2] - (left + w);
	
	pCanvasData->sliderH.fMinVal = fabs(amoutLeft);
	pCanvasData->sliderH.fMaxVal = -fabs(amountRight);

	pCanvasData->sliderV.fMinVal = fabs(amountAbove);
	pCanvasData->sliderV.fMaxVal = -fabs(amountBelow);

	pCanvasData->sliderV.fVal = 0.0f;
	pCanvasData->sliderH.fVal = 0.0f;
}

void* CanvasWidget_OnOutputVerts(struct UIWidget* pWidget, VECTOR(struct WidgetVertex) pOutVerts)
{
	struct CanvasData* pCanvasData = pWidget->pImplementationData;
	bool bExceedsW, bExceedsH;
	if (ContentExceedsSize(pCanvasData, pWidget, &bExceedsW, &bExceedsH))
	{
		GeomRect region;
		GetClipRegion(region, pWidget);
		SetClipRect(region);
		SetSliderPositionAndDims(pWidget, pCanvasData);
		pCanvasData->bVSliderActive = bExceedsH;
		pCanvasData->bHSliderActive = bExceedsW;
	}

	pOutVerts = UI_Helper_OnOutputVerts(pWidget, pOutVerts);

	UnsetClipRect();

	if (bExceedsH)
	{
		pOutVerts = SliderWidget_OnOutputVerts(pOutVerts, &pCanvasData->sliderV, pCanvasData->sliderVTopLeft[1], pCanvasData->sliderVTopLeft[0], &zeroPadding);
	}
	if (bExceedsW)
	{
		pOutVerts = SliderWidget_OnOutputVerts(pOutVerts, &pCanvasData->sliderH, pCanvasData->sliderHTopLeft[1], pCanvasData->sliderHTopLeft[0], &zeroPadding);
	}
	return pOutVerts;
}

static void OnWidgetInit(struct UIWidget* pWidget)
{
	struct CanvasData* pCanvasData = pWidget->pImplementationData;
	LayoutChildren(pWidget, UI_GetWidget(pWidget->hParent));
	GetChildrenBoundingBox(pWidget, pCanvasData->contentBB, &pCanvasData->contentBB[2]);
	bool bExceedsW, bExceedsH;
	if (ContentExceedsSize(pCanvasData, pWidget, &bExceedsW, &bExceedsH))
	{
		// need to call this when the contents of the canvas change
		SetSliderMinAndMax(pWidget, pCanvasData);
	}

}

static void OnPropertyChanged(struct UIWidget* pThisWidget, struct WidgetPropertyBinding* pBinding)
{

}


static void MouseButtonDownCallback(struct UIWidget* pWidget, float x, float y, int btn)
{
	struct CanvasData* pCanvasData = pWidget->pImplementationData;
	
	if (pCanvasData->bHSliderActive)
	{
		GeomRect hsliderRect =
		{
			pCanvasData->sliderHTopLeft[0],
			pCanvasData->sliderHTopLeft[1],
			pCanvasData->sliderHTopLeft[0] + SliderWidget_GetWidth(&pCanvasData->sliderH, &zeroPadding),
			pCanvasData->sliderHTopLeft[1] + SliderWidget_GetHeight(&pCanvasData->sliderH, &zeroPadding) ,
		};
		if (Ge_PointInAABB(x, y, hsliderRect))
		{
			SliderWudget_SetSliderPositionFromMouse(pWidget, &pCanvasData->sliderH, x, y, pCanvasData->sliderHTopLeft[1], pCanvasData->sliderHTopLeft[1], zeroPadding);
			pCanvasData->sliderH.bMouseDown = true;
		}
	}
	if (pCanvasData->bVSliderActive)
	{
		GeomRect vsliderRect =
		{
			pCanvasData->sliderVTopLeft[0],
			pCanvasData->sliderVTopLeft[1],
			pCanvasData->sliderVTopLeft[0] + SliderWidget_GetWidth(&pCanvasData->sliderV, &zeroPadding),
			pCanvasData->sliderVTopLeft[1] + SliderWidget_GetHeight(&pCanvasData->sliderV, &zeroPadding) ,
		};
		if (Ge_PointInAABB(x, y, vsliderRect))
		{
			SliderWudget_SetSliderPositionFromMouse(pWidget, &pCanvasData->sliderV, x, y, pCanvasData->sliderVTopLeft[1], pCanvasData->sliderVTopLeft[1], zeroPadding);
			pCanvasData->sliderV.bMouseDown = true;
		}
	}
}

static void MouseButtonUpCallback(struct UIWidget* pWidget, float x, float y, int btn)
{
	struct CanvasData* pCanvasData = pWidget->pImplementationData;

	pCanvasData->sliderV.bMouseDown = false;
	pCanvasData->sliderH.bMouseDown = false;
}

static void MouseLeaveCallback(struct UIWidget* pWidget, float x, float y)
{
	struct CanvasData* pCanvasData = pWidget->pImplementationData;
	pCanvasData->sliderV.bMouseDown = false;
	pCanvasData->sliderH.bMouseDown = false;
}

static void MouseMoveCallback(struct UIWidget* pWidget, float x, float y)
{
	struct CanvasData* pCanvasData = pWidget->pImplementationData;

	if (pCanvasData->bHSliderActive)
	{
		if (pCanvasData->sliderH.bMouseDown)
		{
			SliderWudget_SetSliderPositionFromMouse(pWidget, &pCanvasData->sliderH, x, y, pCanvasData->sliderHTopLeft[1], pCanvasData->sliderHTopLeft[1], zeroPadding);
		}
	}
	if (pCanvasData->bVSliderActive)
	{
		if (pCanvasData->sliderV.bMouseDown)
		{
			SliderWudget_SetSliderPositionFromMouse(pWidget, &pCanvasData->sliderV, x, y, pCanvasData->sliderVTopLeft[1], pCanvasData->sliderVTopLeft[1], zeroPadding);
		}
	}
}

static void MakeWidgetIntoCanvasWidget(HWidget hWidget, xmlNode* pXMLNode, struct XMLUIData* pUILayerData)
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
	pWidget->fnOutputVertices = &CanvasWidget_OnOutputVerts;
	pWidget->fnOnBoundPropertyChanged = &OnPropertyChanged;
	pWidget->fnOnWidgetInit = &OnWidgetInit;


	pWidget->cCallbacks.Callbacks[WC_OnMouseDown].type = WC_OnMouseDown;
	pWidget->cCallbacks.Callbacks[WC_OnMouseDown].callback.mouseBtnFn = &MouseButtonDownCallback;

	pWidget->cCallbacks.Callbacks[WC_OnMouseUp].type = WC_OnMouseUp;
	pWidget->cCallbacks.Callbacks[WC_OnMouseUp].callback.mouseBtnFn = &MouseButtonUpCallback;

	pWidget->cCallbacks.Callbacks[WC_OnMouseLeave].type = WC_OnMouseLeave;
	pWidget->cCallbacks.Callbacks[WC_OnMouseLeave].callback.mousePosFn = &MouseLeaveCallback;

	pWidget->cCallbacks.Callbacks[WC_OnMouseMove].type = WC_OnMouseMove;
	pWidget->cCallbacks.Callbacks[WC_OnMouseMove].callback.mousePosFn = &MouseMoveCallback;

	pWidget->pImplementationData = malloc(sizeof(struct CanvasData));
	memset(pWidget->pImplementationData, 0, sizeof(struct CanvasData));
	struct CanvasData* pData = pWidget->pImplementationData;
	SliderWidget_MakeDefaultSliderWidget(&pData->sliderH, pUILayerData, SO_Horizontal);
	SliderWidget_MakeDefaultSliderWidget(&pData->sliderV, pUILayerData, SO_Vertical);

}

HWidget CanvasWidgetNew(HWidget hParent, xmlNode* pXMLNode, struct XMLUIData* pUILayerData)
{
	HWidget hWidget = UI_NewBlankWidget();
	MakeWidgetIntoCanvasWidget(hWidget, pXMLNode, pUILayerData);
	return hWidget;
}