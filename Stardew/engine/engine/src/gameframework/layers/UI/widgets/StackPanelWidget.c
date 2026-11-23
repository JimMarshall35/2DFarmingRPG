#include "StackPanelWidget.h"
#include "Widget.h"
#include <stdlib.h>
#include "AssertLib.h"
#include <string.h>
#include "DataNode.h"

void StackPanel_PopulateDataFromXML(struct DataNode* pNode, struct StackPanelWidgetData* pData)
{
	if(pNode->fnGetPropType(pNode, "orientation") == DN_String)
	{
		if(pNode->fnStrCmp(pNode, "orientation", "horizontal"))
		{
			pData->orientation = SPO_Horizontal;
		}
		else if (pNode->fnStrCmp(pNode, "orientation", "vertical"))
		{
			pData->orientation = SPO_Vertical;
		}
		else
		{
			pData->orientation = SPO_Vertical;
		}
	}
}

float StackPanel_GetWidth(struct UIWidget* pWidget, struct UIWidget* pParent, struct StackPanelWidgetData* pStackPanelData)
{
	float width = 0.0;

	if (pWidget->hFirstChild == NULL_HWIDGET)
	{
		return width;
	}
	switch (pStackPanelData->orientation)
	{
	case SPO_Horizontal:
	{
		struct UIWidget* pChild = UI_GetWidget(pWidget->hFirstChild);
		while (pChild)
		{
			width += pChild->fnGetWidth(pChild, pWidget);
			if (pChild->hNext == NULL_HWIDGET)
			{
				pChild = NULL;
			}
			else
			{
				pChild = UI_GetWidget(pChild->hNext);
			}
		}
		break;
	}
	case SPO_Vertical:
	{
		float widest = 0.0;
		struct UIWidget* pChild = UI_GetWidget(pWidget->hFirstChild);
		while (pChild)
		{
			float thisChildW = pChild->fnGetWidth(pChild, pWidget);
			if (thisChildW > widest)
			{
				widest = thisChildW;
			}
			if (pChild->hNext == NULL_HWIDGET)
			{
				pChild = NULL;
			}
			else
			{
				pChild = UI_GetWidget(pChild->hNext);
			}
		}
		width = widest;
		break;
	}
	}
	return width + pWidget->padding.paddingLeft + pWidget->padding.paddingRight;
}

float StackPanel_GetHeight(struct UIWidget* pWidget, struct UIWidget* pParent, struct StackPanelWidgetData* pStackPanelData)
{
	float height = 0.0;

	if (pWidget->hFirstChild == NULL_HWIDGET)
	{
		return height;
	}
	switch (pStackPanelData->orientation)
	{
	case SPO_Horizontal:
	{
		float tallest = 0.0;
		struct UIWidget* pChild = UI_GetWidget(pWidget->hFirstChild);
		while (pChild)
		{
			float thisChildH = pChild->fnGetHeight(pChild, pWidget);
			if (thisChildH > tallest)
			{
				tallest = thisChildH;
			}
			if (pChild->hNext == NULL_HWIDGET)
			{
				pChild = NULL;
			}
			else
			{
				pChild = UI_GetWidget(pChild->hNext);
			}
		}
		height = tallest;
		break;
	}
	case SPO_Vertical:
	{
		struct UIWidget* pChild = UI_GetWidget(pWidget->hFirstChild);
		while (pChild)
		{
			height += pChild->fnGetHeight(pChild, pWidget);
			if (pChild->hNext == NULL_HWIDGET)
			{
				pChild = NULL;
			}
			else
			{
				pChild = UI_GetWidget(pChild->hNext);
			}
		}
		break;
	}
	}
	return height + pWidget->padding.paddingTop + pWidget->padding.paddingBottom;
}

void StackPanel_LayoutChildren(struct UIWidget* pWidget, struct UIWidget* pParent, struct StackPanelWidgetData* pStackPanelData, float w, float h)
{
	if (pWidget->hFirstChild == NULL_HWIDGET)
	{
		return;
	}

	float top = pWidget->top + pWidget->padding.paddingTop;
	float left = pWidget->left + pWidget->padding.paddingLeft;


	switch (pStackPanelData->orientation)
	{
	case SPO_Horizontal:
	{
		struct UIWidget* pChild = UI_GetWidget(pWidget->hFirstChild);
		while (pChild)
		{
			float childHeight = pChild->fnGetHeight(pChild, pWidget);
			if (childHeight == h)
			{
				pChild->left = left;
				pChild->top = top;
				left += pChild->fnGetWidth(pChild, pWidget);
			}
			else
			{
				EASSERT(childHeight < h);
				switch(pChild->verticalAlignment)
				{
				case WVA_Middle:
					pChild->top = top + (h - childHeight) * 0.5;
					break;
				case WVA_Bottom:
					pChild->top = top + (h - childHeight);
					break;
				case WVA_Top:
					pChild->top = top;
					break;
				default:
					EASSERT(false);
					break;
				}
				pChild->left = left;
				left += pChild->fnGetWidth(pChild, pWidget);

			}

			if (pChild->hNext == NULL_HWIDGET)
			{
				pChild = NULL;
			}
			else
			{
				pChild = UI_GetWidget(pChild->hNext);
			}
		}

		break;
	}
	case SPO_Vertical:
	{
		struct UIWidget* pChild = UI_GetWidget(pWidget->hFirstChild);
		while (pChild)
		{
			float childWidth = pChild->fnGetWidth(pChild, pWidget);
			if (childWidth == w)
			{
				pChild->left = left;
				pChild->top = top;
				top += pChild->fnGetHeight(pChild, pWidget);
			}
			else
			{
				EASSERT(childWidth < w);
				switch(pChild->horizontalAlignment)
				{
				case WHA_Left:
					pChild->left = left;
					break;
				case WHA_Middle:
					pChild->left = left + (w - childWidth) * 0.5f;
					break;
				case WHA_Right:
					pChild->left = left + (w - childWidth);
					break;
				}
				pChild->top = top;
				top += pChild->fnGetHeight(pChild, pWidget);
			}

			if (pChild->hNext == NULL_HWIDGET)
			{
				pChild = NULL;
			}
			else
			{
				pChild = UI_GetWidget(pChild->hNext);
			}
		}
		break;
	}
	}
	UI_Helper_OnLayoutChildren(pWidget, pParent);
}

static float GetWidth(struct UIWidget* pWidget, struct UIWidget* pParent)
{
	return StackPanel_GetWidth(pWidget, pParent, pWidget->pImplementationData);
}

static float GetHeight(struct UIWidget* pWidget, struct UIWidget* pParent)
{
	return StackPanel_GetHeight(pWidget, pParent, pWidget->pImplementationData);
}

static void LayoutChildren(struct UIWidget* pWidget, struct UIWidget* pParent)
{
	struct StackPanelWidgetData* pStackPanelData = pWidget->pImplementationData;
	StackPanel_LayoutChildren(pWidget, pParent, pStackPanelData, GetWidth(pWidget, pParent), GetHeight(pWidget, pParent));
}

static void OnDestroy(struct UIWidget* pWidget)
{
	assert(pWidget->pImplementationData);
	free(pWidget->pImplementationData);
}

static void* OnOutputVerts(struct UIWidget* pWidget, VECTOR(WidgetVertex) pOutVerts)
{
	pOutVerts = UI_Helper_OnOutputVerts(pWidget, pOutVerts);
	return pOutVerts;
}

static void MakeWidgetIntoStackPanel(HWidget hWidget, struct DataNode* pNode)
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
	pWidget->pImplementationData = malloc(sizeof(struct StackPanelWidgetData));
	memset(pWidget->pImplementationData, 0, sizeof(struct StackPanelWidgetData));
	struct StackPanelWidgetData* pData = pWidget->pImplementationData;
	StackPanel_PopulateDataFromXML(pNode, pData);
}

HWidget StackPanelWidgetNew(HWidget hParent, struct DataNode* pNode, struct XMLUIData* pUIData)
{
	HWidget hWidget = UI_NewBlankWidget();
	MakeWidgetIntoStackPanel(hWidget, pNode);
	return hWidget;
}
