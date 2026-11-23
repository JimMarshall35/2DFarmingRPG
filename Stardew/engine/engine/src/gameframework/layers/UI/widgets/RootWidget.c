#include "RootWidget.h"
#include "Widget.h"
#include "main.h"
#include <string.h>        
#include <stdlib.h>
#include "AssertLib.h"
#include "Profile.h"

float RootWidget_GetWidth(struct UIWidget* pWidget, struct UIWidget* pParent)
{
	return ((struct RootWidgetData*)pWidget->pImplementationData)->windowW;
}

float RootWidget_GetHeight(struct UIWidget* pWidget, struct UIWidget* pParent)
{
	return ((struct RootWidgetData*)pWidget->pImplementationData)->windowH;
}

void RootWidget_LayoutChildren(struct UIWidget* pThis, struct UIWidget* pParent, float scrollX, float scrollY)
{
	ZONE(ctx)
	if (pThis->hFirstChild != NULL_HWIDGET)
	{
		struct UIWidget* pWidget = UI_GetWidget(pThis->hFirstChild);
		float windowW = UI_ResolveWidthDimPxls(pThis, &pThis->width);
		float windowH = UI_ResolveHeightDimPxls(pThis, &pThis->height);
		while (pWidget)
		{
			float width = pWidget->fnGetWidth(pWidget, pThis);
			float height = pWidget->fnGetHeight(pWidget, pThis);

			switch (pWidget->dockPoint)
			{
			case WDP_TopLeft:
				pWidget->top = pThis->top;
				pWidget->left = pThis->left;
				break;
			case WDP_TopMiddle:
				pWidget->top = pThis->top;
				pWidget->left = pThis->left + ((windowW / 2) - (width / 2));
				break;
			case WDP_TopRight:
				pWidget->top = pThis->top;
				pWidget->left = pThis->left + (windowW - width);
				break;
			case WDP_MiddleRight:
				pWidget->top = pThis->top + ((windowH / 2.0f) - (height / 2.0f));
				pWidget->left = pThis->left + (windowW - width);
				break;
			case WDP_BottomRight:
				pWidget->top = pThis->top + (windowH - height);
				pWidget->left = pThis->left + (windowW - width);
				break;
			case WDP_BottomMiddle:
				pWidget->top = pThis->top + (windowH - height);
				pWidget->left = pThis->left + (windowW / 2.0f - width / 2.0f);
				break;
			case WDP_BottomLeft:
				pWidget->top = pThis->top + (windowH - height);
				pWidget->left = pThis->left;
				break;
			case WDP_MiddleLeft:
				pWidget->top = pThis->top + (windowH / 2.0f) - (height / 2.0f);
				pWidget->left = pThis->left;
				break;
			case WDP_Centre:
				pWidget->top = pThis->top + (windowH / 2.0f) - (height / 2.0f);
				pWidget->left = pThis->left + (windowW / 2.0f) - (width / 2.0f);
				break;
			}

			pWidget->left += pWidget->offsetX;
			pWidget->top += pWidget->offsetY;

			pWidget->left += scrollX;
			pWidget->top += scrollY;


			pWidget->fnLayoutChildren(pWidget, pThis);
			if (pWidget->hNext != NULL_HWIDGET)
			{
				pWidget = UI_GetWidget(pWidget->hNext);
			}
			else
			{
				pWidget = NULL;
			}
		}
	}
	ZONE_END(ctx)
}

void RootWidget_OnDestroy(struct UIWidget* pThis)
{
	ZONE(ctx)
	free(pThis->pImplementationData);
	ZONE_END(ctx)
}

static void* OnOutputVerts(struct UIWidget* pWidget, VECTOR(WidgetVertex) pOutVerts)
{
	ZONE(ctx)
	void* p = UI_Helper_OnOutputVerts(pWidget, pOutVerts);
	ZONE_END(ctx)
	return p;
}

static void LayoutChildren(struct UIWidget* pThis, struct UIWidget* pParent)
{
	ZONE(ctx)
	RootWidget_LayoutChildren(pThis, pParent, 0.0f, 0.0f);
	ZONE_END(ctx)
}

void MakeIntoRootWidget(HWidget widget)
{
	ZONE(ctx)
	struct UIWidget* pWidget = UI_GetWidget(widget);
	struct RootWidgetData* pRootWidgetData = malloc(sizeof(struct RootWidgetData));
	memset(pRootWidgetData, 0, sizeof(struct RootWidgetData));
	pRootWidgetData->windowW = Mn_GetScreenWidth();
	pRootWidgetData->windowH = Mn_GetScreenHeight();
	pRootWidgetData->bIsDirty = true;

	pWidget->pImplementationData = pRootWidgetData;
	pWidget->fnGetWidth = &RootWidget_GetWidth;
	pWidget->fnGetHeight = &RootWidget_GetHeight;
	pWidget->fnLayoutChildren = &LayoutChildren;
	pWidget->fnOnDestroy = &RootWidget_OnDestroy;
	pWidget->fnOutputVertices = &OnOutputVerts;
	pWidget->left = 0.0f;
	pWidget->top = 0.0f;

	pWidget->width.type = WD_Pixels;
	pWidget->width.data = pRootWidgetData->windowW;
	pWidget->height.type = WD_Pixels;
	pWidget->height.data = pRootWidgetData->windowH;
	ZONE_END(ctx)
}

HWidget NewRootWidget()
{
	ZONE(ctx)
	HWidget w = UI_NewBlankWidget();
	MakeIntoRootWidget(w);
	ZONE_END(ctx)
	return w;
}

void RootWidget_OnWindowSizeChanged(HWidget rootWidget, int newW, int newH)
{
	ZONE(ctx)
	struct UIWidget* pWidget = UI_GetWidget(rootWidget);
	struct RootWidgetData* pRootData = pWidget->pImplementationData;
	pRootData->windowH = newH;
	pRootData->windowW = newW;
	pWidget->width.data = newW;
	pWidget->height.data = newH;
	pRootData->bIsDirty = true;
	ZONE_END(ctx)
}
bool GetRootWidgetIsDirty(HWidget hRootWidget)
{
	ZONE(ctx)
	struct UIWidget* pWidget = UI_GetWidget(hRootWidget);
	struct RootWidgetData* pRootData = pWidget->pImplementationData;
	ZONE_END(ctx)
	return pRootData->bIsDirty;
}

// if dirty, a whole tree rebuild will be triggered
void SetRootWidgetIsDirty(HWidget hRootWidget, bool bIsDirty)
{
	ZONE(ctx)
	struct UIWidget* pWidget = UI_GetWidget(hRootWidget);
	struct RootWidgetData* pRootData = pWidget->pImplementationData;
	pRootData->bIsDirty = bIsDirty;
	ZONE_END(ctx)
}

