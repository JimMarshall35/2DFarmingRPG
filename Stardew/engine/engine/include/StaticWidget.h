#ifndef STATIC_WIDGET_H
#define STATIC_WIDGET_H

#include "HandleDefs.h"
#include "DynArray.h"
#include "Widget.h"

struct Vert2DColourTexture;
typedef struct Vert2DColourTexture WidgetVertex;


struct DataNode;

struct StaticWidgetData
{
	char* imageName;
	struct WidgetScale scale;
	hSprite sprite;
	hAtlas atlas;
};

struct XMLUIData;

HWidget StaticWidgetNew(HWidget hParent, struct DataNode* pXMLNode, struct XMLUIData* pUILayerData);

void* StaticWidget_OnOutputVerts(struct StaticWidgetData* pStaticData, float left, float top, struct WidgetPadding* pPadding, VECTOR(WidgetVertex) pOutVerts);
void StaticWidget_Destroy(struct StaticWidgetData* pStaticData);
float StaticWidget_GetHeight(struct StaticWidgetData* pStaticData, struct WidgetPadding* pPadding);
float StaticWidget_GetWidth(struct StaticWidgetData* pStaticData, struct WidgetPadding* pPadding);
void StaticWidget_MakeFromXML(struct StaticWidgetData* pWidgetData, struct DataNode* pXMLNode, struct XMLUIData* pUILayerData);

#endif // !STATIC_WIDGET_H
