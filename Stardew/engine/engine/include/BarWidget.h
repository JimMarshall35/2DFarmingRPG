#ifndef BARWIDGET_H
#define BARWIDGET_H

#include <stdbool.h>
#include "HandleDefs.h"
#include "StaticWidget.h"

struct Vert2DColourTexture;
typedef struct Vert2DColourTexture WidgetVertex;

struct XMLUIData;
struct DataNode;

enum BarOrientation
{
    BO_Undefined,
    BO_Horizontal,
    BO_Vertical,
};


enum BarFillAnchorPt
{
    /*
        When the filled portion of the bar is < 100%,
        the empty portion of the bar is at the bottom
        in the case of a vertical bar or to the right
        in the case of a horizontal bar.
    */
    AP_TopOrLeft,

    /*
        When the filled portion of the bar is < 100%,
        the empty portion of the bar is distributed evenly
        either side
    */
    AP_Middle,

    /*
        When the filled portion of the bar is < 100%,
        the empty portion of the bar is at the top
        in the case of a vertical bar or to the left
        in the case of a horizontal bar.
    */
    AP_RightOrBottom,
};

struct BarData
{
    enum BarFillAnchorPt fillAnchorPt;
    enum BarOrientation orientation;
    struct StaticWidgetData frameStaticData;
    struct StaticWidgetData fillStaticData;

    /* between 0 and 1 */
    float fVal;

    float lengthPx;

    HWidget rootWidget;
};

HWidget BarWidgetNew(HWidget hParent, struct DataNode* pXMLNode, struct XMLUIData* pUILayerData);


#endif
