#ifndef XMLUI_GAME_LAYER_H
#define XMLUI_GAME_LAYER_H

#include <libxml/tree.h>


#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "HandleDefs.h"
#include "DynArray.h"
#include "Widget.h"
#include "DrawContext.h"
#include "TimerPool.h"
#include "InputContext.h"

struct XMLUIData;
struct GameFrameworkLayer;
struct DataNode;

struct XMLUIGameLayerOptions
{
	/// @brief draw context (should really just be globally accessible: TODO: fix)
	DrawContext* pDc;

	/// @brief path to xml file defining the UI
	const char* xmlPath;

	/// @brief eg, when XMLUIGameLayer_Get is called, do we load the xml file?
	bool bLoadImmediately;
};

/// @brief Function pointer typedef for adding a new widget, described by pDataNode as a child of hParent
/// should return a handle to the created widget
typedef HWidget(*AddChildFn)(HWidget hParent, struct DataNode* pDataNode, struct XMLUIData* pUILayerData);

#define XML_UI_MAX_PATH 256
#define MAX_FOCUSED_WIDGETS 16


/// @brief its a "request" because if an ancenstor of the requesting widget 
/// also requests then the request is overruled
struct WidgetChildrenChangeRequest
{
	/// @brief Registry index of the viewmodel table the function will be called on
	int regIndex;

	/// @brief Function that will generate new children for the widget
	const char* funcName;

	/// @brief Widget that will have new children
	HWidget hWidget;
};


typedef struct XMLUIData
{
	HWidget rootWidget;
	char xmlFilePath[XML_UI_MAX_PATH];
	const char* xmlData;
	bool bLoaded;
	hAtlas atlas;
	VECTOR(WidgetVertex) pWidgetVertices;
	HUIVertexBuffer hVertexBuffer;
	int hViewModel; // reference to lua table
	HWidget focusedWidgets[MAX_FOCUSED_WIDGETS];
	int nFocusedWidgets;
	struct SDTimerPool timerPool;
	VECTOR(struct WidgetChildrenChangeRequest) pChildrenChangeRequests;
	struct AxisBinding gMouseX;
	struct AxisBinding gMouseY;
	struct ButtonBinding gMouseBtnLeft;
}XMLUIData;


/// @brief Get and possibly load (if bLoadImmediately) a ui layer defined by xml
/// @param pLayer (Out) layer to turn into an xml ui layer
/// @param pOptions options for creation
void XMLUIGameLayer_Get(struct GameFrameworkLayer* pLayer, struct XMLUIGameLayerOptions* pOptions);

void XMLUI_PushGameFrameworkLayer(const char* xmlPath);

#ifdef __cplusplus
}
#endif

#endif
