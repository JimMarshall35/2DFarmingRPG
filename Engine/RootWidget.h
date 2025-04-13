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

// if dirty, a whole tree rebuild will be triggered
void SetRootWidgetIsDirty(HWidget hRootWidget, bool bIsDirty);

HWidget NewRootWidget();

void RootWidget_OnWindowSizeChanged(HWidget rootWidget, int newW, int newH);

#endif