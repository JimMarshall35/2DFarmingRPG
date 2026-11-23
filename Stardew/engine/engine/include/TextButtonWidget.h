#ifndef TEXTBUTTONWIDGET_H
#define TEXTBUTTONWIDGET_H

#include "HandleDefs.h"

struct XMLUIData;
struct DataNode;

/// @brief Create a new button that displays a string of text
/// @param hParent 
/// @param pXMLNode 
/// @param pUILayerData 
/// @return 
HWidget TextButtonWidgetNew(HWidget hParent, struct DataNode* pXMLNode, struct XMLUIData* pUILayerData);


#endif // !TEXTBUTTONWIDGET_H

