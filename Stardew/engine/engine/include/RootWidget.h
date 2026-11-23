#ifndef ROOTWIDGET_H
#define ROOTWIDGET_H
#include "Widget.h"


struct RootWidgetData
{
	int windowW;
	int windowH;
	bool bIsDirty;
};

bool GetRootWidgetIsDirty(HWidget hRootWidget);

/// @brief  if dirty, a whole tree rebuild will be triggered
/// @param hRootWidget 
/// @param bIsDirty 
void SetRootWidgetIsDirty(HWidget hRootWidget, bool bIsDirty);

HWidget NewRootWidget();

void RootWidget_OnWindowSizeChanged(HWidget rootWidget, int newW, int newH);

void RootWidget_LayoutChildren(struct UIWidget* pThis, struct UIWidget* pParent, float scrollX, float scrollY);

#endif