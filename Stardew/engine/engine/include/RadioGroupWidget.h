#ifndef RADIOGROUPWIDGET_H
#define RADIOGROUPWIDGET_H

#include "HandleDefs.h"

struct XMLUIData;
struct UIWidget;
struct DataNode;

HWidget RadioGroupWidgetNew(HWidget hParent, struct DataNode* pXMLNode, struct XMLUIData* pUILayerData);
void RadioGroup_ChildSelected(HWidget hRadioGroup, struct UIWidget* pRadioButtonChild);


#endif // !RADIOGROUPWIDGET_H
