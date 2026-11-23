#ifndef CANVASWIDGET_H
#define CANVASWIDGET_H

#include "HandleDefs.h"

/**
	@file CanvasWidget.h
	@brief
	A widget with fixed dimensions, optionally set in terms of its parents dims.
	Arranges its children according to docking position
*/
#include <stdbool.h>
#include "Geometry.h"
#include "SliderWidget.h"

struct CanvasData
{
	float scrollX;
	float scrollY;
	GeomRect contentBB;

	struct SliderData sliderH;
	struct SliderData sliderV;
	vec2 sliderHTopLeft;
	vec2 sliderVTopLeft;

	bool bHSliderActive;
	bool bVSliderActive;

	bool bUseHSlider;
	bool bUseVSlider;
};

struct DataNode;
struct XMLUIData;
struct UIWidget;
struct Vert2DColourTexture;
typedef struct Vert2DColourTexture WidgetVertex;

HWidget CanvasWidgetNew(HWidget hParent, struct DataNode* pXMLNode, struct XMLUIData* pUILayerData);
void* CanvasWidget_OnOutputVerts(struct UIWidget* pWidget, WidgetVertex* pOutVerts);


#endif // !CANVASWIDGET_H
