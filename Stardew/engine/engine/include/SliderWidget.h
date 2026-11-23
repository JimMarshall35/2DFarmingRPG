#ifndef SLIDERWIDGET_H
#define SLIDERWIDGET_H

#include "HandleDefs.h"
#include "DynArray.h"
#include <stdbool.h>
#include "StaticWidget.h"

struct Vert2DColourTexture;
typedef struct Vert2DColourTexture WidgetVertex;

enum SliderOrientation
{
	SO_Vertical,
	SO_Horizontal
};

struct SliderData
{
	enum SliderOrientation orientation;
	struct StaticWidgetData railStaticData;
	struct StaticWidgetData sliderStaticData;
	float fVal;
	float fMaxVal;
	float fMinVal;
	float lengthPx;
	HWidget rootWidget;
	bool bMouseDown;
};
struct XMLUIData;
struct DataNode;

void SliderWidget_MakeDefaultSliderWidget(struct SliderData* pData, struct XMLUIData* pUILayerData, enum SliderOrientation orientation);

HWidget SliderWidgetNew(HWidget hParent, struct DataNode* pXMLNode, struct XMLUIData* pUILayerData);

void* SliderWidget_OnOutputVerts(VECTOR(WidgetVertex) pOutVerts, struct SliderData* pData, float top, float left, struct WidgetPadding* pPadding);

void SliderWudget_SetSliderPositionFromMouse(struct UIWidget* pWidget, struct SliderData* pData, float x, float y, float top, float left, struct WidgetPadding padding);

float SliderWidget_GetWidth(struct SliderData* pData, struct WidgetPadding* pPadding);

float SliderWidget_GetHeight(struct SliderData* pData, struct WidgetPadding* pPadding);

#endif // !SLIDERWIDGET_H
