#ifndef TEXTENTRYWIDGET_H
#define TEXTENTRYWIDGET_H

#include "HandleDefs.h"

/*

A widget for entering text


*/
struct DataNode;
struct XMLUIData;

/// @brief Create a new widget for entering text from a struct DataNode*
/// @param hParent 
/// @param pXMLNode 
/// @param pUILayerData 
/// @return 
HWidget TextEntryWidgetNew(HWidget hParent, struct DataNode* pXMLNode, struct XMLUIData* pUILayerData);


#endif // !TEXTENTRYWIDGET_H
