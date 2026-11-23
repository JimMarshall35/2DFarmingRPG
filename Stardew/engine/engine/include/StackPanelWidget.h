#ifndef STACKPANEL_WIDGET_H
#define STACKPANEL_WIDGET_H
#include "HandleDefs.h"

typedef struct _xmlNode xmlNode;

struct Vert2DColourTexture;
typedef struct Vert2DColourTexture WidgetVertex;

typedef enum
{
	SPO_Horizontal,
	SPO_Vertical
}StackPanelOrientation;

struct StackPanelWidgetData
{
	StackPanelOrientation orientation;
};

struct UIWidget;
struct XMLUIData;

struct DataNode;

HWidget StackPanelWidgetNew(HWidget hParent, struct DataNode* pXMLNode, struct XMLUIData* pUIData);

void StackPanel_PopulateDataFromXML(struct DataNode* pXMLNode, struct StackPanelWidgetData* pData);
float StackPanel_GetWidth(struct UIWidget* pWidget, struct UIWidget* pParent, struct StackPanelWidgetData* pStackPanelData);
float StackPanel_GetHeight(struct UIWidget* pWidget, struct UIWidget* pParent, struct StackPanelWidgetData* pStackPanelData);
void StackPanel_LayoutChildren(struct UIWidget* pWidget, struct UIWidget* pParent, struct StackPanelWidgetData* pStackPanelData, float w, float h);



#define STACKPANEL_XML_NAME "stackpanel"

#endif
