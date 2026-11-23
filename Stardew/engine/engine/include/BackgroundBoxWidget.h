#ifndef BACKGROUNDBOXWIDGET_H
#define BACKGROUNDBOXWIDGET_H

#include "HandleDefs.h"
#include "Widget.h"

struct BackgroundBoxWidgetData
{
	char* imageName;
	struct WidgetScale scale;
	hSprite sprite;
	hAtlas atlas;
};

struct XMLUIData;
struct DataNode;

void BackgroundBoxWidget_fromXML(struct UIWidget* pWidget, struct BackgroundBoxWidgetData* pWidgetData, struct DataNode* pXMLNode, struct XMLUIData* pUILayerData);
HWidget BackgroundBoxWidgetNew(HWidget hParent, struct DataNode* pXMLNode, struct XMLUIData* pUILayerData);
void* BackgroundBoxWidget_OutputVerts(
	struct BackgroundBoxWidgetData* pBBoxData,
	VECTOR(WidgetVertex) pOutVerts,
	float totalWidth,
	float totalHeight,
	const struct WidgetPadding* padding,
	float left, float top
);

void BackgroundBoxWidget_Destroy(struct BackgroundBoxWidgetData* pBBoxData);


#endif // !BACKGROUNDBOXWIDGET_H

