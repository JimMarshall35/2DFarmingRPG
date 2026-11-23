#ifndef RADIOBUTTONWIDGET_H
#define RADIOBUTTONWIDGET_H

#include "HandleDefs.h"
#include <stdbool.h>

struct DataNode;
struct XMLUIData;

HWidget RadioButtonWidgetNew(HWidget hParent, struct DataNode* pXMLNode, struct XMLUIData* pUILayerData);

void RadioButton_SetSelected(HWidget hRadioButton, bool bSelected);

#endif // !RADIOBUTTONWIDGET_H
