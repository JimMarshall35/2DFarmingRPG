#ifndef  TEXTWIDGET_H
#define TEXTWIDGET_H

#include "HandleDefs.h"
#include "DynArray.h"
#include <stdbool.h>

typedef struct _AtlasSprite AtlasSprite;

struct DataNode;

#define TEXT_WIDGET_DATA_LUA_CALLBACK_NAME_BUFFER_SIZE 32

struct TextWidgetData
{
	char* content;
	HFont font;
	float fSizePts;
	hAtlas atlas;
	float r, g, b, a;
	HWidget rootWidget;
	char onEnterPressLuaCallbackName[TEXT_WIDGET_DATA_LUA_CALLBACK_NAME_BUFFER_SIZE];
	bool bEnterPressCallbackSet;
};


struct XMLUIData;
struct WidgetPadding;
struct WidgetVertex;
struct UIWidget;

void TextWidget_FromXML(struct UIWidget* pWidget, struct TextWidgetData* pData, struct DataNode* pXMLNode, struct XMLUIData* pUILayerData);

HWidget TextWidgetNew(HWidget hParent, struct DataNode* pXMLNode, struct XMLUIData* pUILayerData);

void* TextWidget_OutputVerts(float left, float top, const struct WidgetPadding* padding, struct TextWidgetData* pData, VECTOR(struct WidgetVertex) pOutVerts);

void TextWidget_Destroy(struct TextWidgetData* pData);

// overlay a charcter at a given letter in the string
void* TextWidget_OutputAtLetter(float left, float top, const struct WidgetPadding* padding, struct TextWidgetData* pData, char charOverlay, int letterOverlay, VECTOR(struct WidgetVertex) pOutVerts);

#endif // ! TEXTWIDGET_H
