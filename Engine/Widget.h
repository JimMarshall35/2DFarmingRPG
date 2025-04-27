#ifndef WIDGET_H
#define WIDGET_H

#include "ObjectPool.h"
#include "HandleDefs.h"
#include <stdbool.h>
#include "DynArray.h"
#include "Geometry.h"

struct UIWidget;

struct xml_string;
struct xml_node;

struct WidgetVertex
{
	float x, y;
	float u, v;
	float r, g, b, a;
};

typedef void(*PrintfFn)(const char* fmt, ...);

typedef float(*GetUIWidgetDimensionFn)(struct UIWidget* pWidget, struct UIWidget* pParent);
typedef void(*LayoutChildrenFn)(struct UIWidget* pWidget, struct UIWidget* pParent);
typedef void(*OnDestroyWidgetFn)(struct UIWidget* pWidget);
typedef void(*OnDebugPrintFn)(int indentLvl, struct UIWidget* pWidget, PrintfFn printfFn);
typedef void*(*OutputWidgetVerticesFn)(struct UIWidget* pThisWidget, VECTOR(struct WidgetVertex) pOutVerts);
typedef void(*OnWidgetInitFn)(struct UIWidget* pWidget);
typedef void(*OnBoundPropertyChangedFn)(struct UIWidget* pThisWidget, struct WidgetPropertyBinding* pBinding);

struct WidgetPadding
{
	float paddingTop;
	float paddingBottom;
	float paddingLeft;
	float paddingRight;
};

struct WidgetScale
{
	float scaleX;
	float scaleY;
};

enum WidgetDimType
{
	WD_Auto,
	WD_Stretch,
	FD_Pixels,
	FD_ScreenFraction,
};

typedef enum WidgetHorizontalAlignment
{
	WHA_Left,
	WHA_Middle,
	WHA_Right
}WidgetHorizontalAlignment;

typedef enum WidgetVerticalAlignment
{
	WVA_Top,
	WVA_Middle,
	WVA_Bottom
}WidgetVerticalAlignment;

typedef enum WidgetDockPoint
{
	WDP_TopLeft,
	WDP_TopMiddle,
	WDP_TopRight,
	WDP_MiddleRight,
	WDP_BottomRight,
	WDP_BottomMiddle,
	WDP_BottomLeft,
	WDP_MiddleLeft,
	WDP_Centre
}WidgetDockPoint;

struct WidgetDim
{
	enum WidgetDimType type;
	float data;
};



#define LUA_CALLBACK_MAX_NAME_LEN 64
struct LuaWidgetCallback
{
	char name[LUA_CALLBACK_MAX_NAME_LEN + 1];
	size_t nameLen;
	bool bActive;
};

enum WidgetCallbackTypes
{
	WC_OnMouseEnter,
	WC_OnMouseLeave,
	WC_OnMouseMove,
	WC_OnMouseDown,
	WC_OnMouseUp,
	WC_NUM
};

typedef void (*WidgetMousePosCallbackFn)(struct UIWidget* pWidget, float x, float y);
typedef void (*WidgetMouseBtnCallbackFn)(struct UIWidget* pWidget, float x, float y, int btn);


/// <summary>
/// this isn't some "hungarian notation" shit - the name refers to widget callbacks defined as C functions!
/// </summary>
struct CWidgetMouseCallback
{
	enum WidgetCallbackTypes type;
	union
	{
		WidgetMousePosCallbackFn mousePosFn;
		WidgetMouseBtnCallbackFn mouseBtnFn;
		
		/*
			here, check this out...
			A random value we can check is zero to determine if the callback is set
		*/
		u64 bActive;
	}callback;
};


struct CWidgetCallbacks
{
	struct CWidgetMouseCallback Callbacks[WC_NUM];

};

struct LuaWidgetCallbacks
{
	int viewmodelTable;
	struct LuaWidgetCallback Callbacks[WC_NUM];
};

// todo: make somehow glue this to the input system or change the input system to fit this 
#define NUM_BUTTONS 3
#define MAX_PROPERTY_NAME_LEN 32
#define MAX_NUM_BINDINGS 16

enum WidgetPropertyBindingType
{
	WBT_Int,
	WBT_Float,
	WBT_String,
	WBT_Bool,
};

struct WidgetPropertyBinding
{
	enum WidgetPropertyBindingType type;
	union
	{
		int i;
		float flt;
		char* str;
		bool bl;
	}data;
	char name[MAX_PROPERTY_NAME_LEN + 1];             // attribute content
	char boundPropertyName[MAX_PROPERTY_NAME_LEN + 1];// attribute name
};

struct WidgetMouseInfo
{
	float x;
	float y;
	int button;
};

struct UIWidget
{
	HWidget hNext;
	HWidget hPrev;
	HWidget hParent;
	HWidget hFirstChild;
	void* pImplementationData;
	GetUIWidgetDimensionFn fnGetWidth;
	GetUIWidgetDimensionFn fnGetHeight;
	LayoutChildrenFn fnLayoutChildren;
	OnDestroyWidgetFn fnOnDestroy;
	OnDebugPrintFn fnOnDebugPrint;
	OutputWidgetVerticesFn fnOutputVertices;
	OnBoundPropertyChangedFn fnOnBoundPropertyChanged;
	OnWidgetInitFn fnOnWidgetInit; // called once whole widget tree is constructed
	float top;
	float left;
	WidgetDockPoint dockPoint;
	struct WidgetPadding padding;
	struct LuaWidgetCallbacks scriptCallbacks;
	struct CWidgetCallbacks cCallbacks;
	struct WidgetPropertyBinding bindings[MAX_NUM_BINDINGS];
	size_t numBindings;
};


void UI_Init();


void UI_DestroyWidget(HWidget widget);


HWidget UI_NewBlankWidget();

size_t UI_CountWidgetChildrenPtr(struct UIWidget* pWidget);

size_t UI_CountWidgetChildren(HWidget pWidget);

struct UIWidget* UI_GetWidget(HWidget hWidget);

void UI_AddChild(HWidget hParent, HWidget hChild);

void UI_ParseWidgetDimsAttribute(struct xml_string* contents, struct WidgetDim* outWidgetDims);

void UI_ParseWidgetPaddingAttributes(struct xml_node* pInNode, struct WidgetPadding* outWidgetPadding);

void UI_ParseHorizontalAlignementAttribute(struct xml_string* contents, enum WidgetHorizontalAlignment* outAlignment);

void UI_ParseVerticalAlignementAttribute(struct xml_string* contents, enum WidgetVerticalAlignment* outAlignment);

float UI_ResolveWidgetDimPxls(const struct WidgetDim* dim);

bool UI_ParseWidgetDockPoint(struct xml_node* pInNode, struct UIWidget* outWidget);

/// <summary>
/// Init common attributes that all widgets can have
///		- "the base class constructor"
/// </summary>
/// <param name="pInNode"></param>
/// <param name="outWidget"></param>
void UI_WidgetCommonInit(struct xml_node* pInNode, struct UIWidget* outWidget);

/// <summary>
/// print a text representation of the widget tree.
/// Falls appart if a widget fails to implement its debug print hook correctly.
/// Shit.
/// </summary>
/// <param name="inWidget"></param>
/// <param name="pPrintfFn"></param>
void UI_DebugPrintCommonWidgetInfo(const struct UIWidget* inWidget, PrintfFn pPrintfFn);

void* UI_Helper_OnOutputVerts(struct UIWidget* pWidget, VECTOR(struct WidgetVertex) pOutVerts);

void UI_Helper_OnLayoutChildren(struct UIWidget* pWidget, struct UIWidget* pParent);

void UI_GetWidgetSize(HWidget hWidget, float* pOutW, float* pOutH);

/// <summary>
/// get top left BEFORE padding is applied (the raw top left)
/// </summary>
/// <param name="hWidget"></param>
/// <param name="pOutLeft"></param>
/// <param name="pOutTop"></param>
void UI_GetWidgetTopLeft(HWidget hWidget, float* pOutLeft, float* pOutTop);

void UI_GetWidgetPadding(HWidget hWidget, float* pOutPaddingTop, float* pOutPaddingBottom, float* pOutPaddingLeft, float* pOutPaddingRight);

/// <summary>
/// send a mouse event to the widget that will be handled either by either or both a lua function and a 
/// c function. If both the C function is called first
/// </summary>
/// <param name="pWidget"></param>
/// <param name=""></param>
/// <param name="pMouseInfo">
///		data about the event
/// </param>
void UI_SendWidgetMouseEvent(struct UIWidget* pWidget, enum WidgetCallbackTypes type, struct WidgetMouseInfo* pMouseInfo);

/// <summary>
/// Get the AABB of the widget, excluding padding.
/// </summary>
/// <param name="outRect"></param>
/// <param name="hWidget"></param>
void UI_GetHitBox(GeomRect outRect, HWidget hWidget);

/// <summary>
/// is the value of an xml attribute property
/// an absolute value (return false) or is it deferrred to
/// a getter on the lua viewmodel (return true).
/// Special syntax for this is to surround the name of a view model property with "{" and "}"
/// </summary>
bool UI_IsAttributeStringABindingExpression(const char* attributeValue);

/// <summary>
/// Add a bound string property entry to the widget
/// and initialise a value by calling the binding getter function
/// </summary>
/// <param name="pWidget">
///		theWidget to add the property binding. When the value changes 
///		the property of the widget will change when OnPropertyChanged is called.
///     2 way bindings will also be introduced for sliders, text entry fields, ect
/// </param>
/// <param name="inBoundPropertyName">
///		bound property name - as it appears in the viewmodel lua table
/// </param>
/// <param name="inBindingExpression">
///		the incoming binding expression, i.e. {spritename}
/// </param>
/// <param name="pOutData">
///		the value returned from calling the lua string getter - allocates a string setting the pointer.
///		user must deallocate
/// </param>
/// <param name="viewmodelTableIndex">
///		the index in the lua registry that contains the lua table containing the function to call
/// </param>
void UI_AddStringPropertyBinding(struct UIWidget* pWidget, char* inBoundPropertyName, char* inBindingExpression, char** pOutData, int viewmodelTableIndex);

void UI_AddIntPropertyBinding(struct UIWidget* pWidget, char* inBoundPropertyName, char* inBindingExpression, int* pOutData, int viewmodelTableIndex);

void UI_AddFloatPropertyBinding(struct UIWidget* pWidget, char* inBoundPropertyName, char* inBindingExpression, int* pOutData, int viewmodelTableIndex);

/// <summary>
/// USER MUST FREE RETURNED PTR
/// append Get_ to the start of the string.
/// Gett
/// </summary>
/// <param name="inBindingName"></param>
/// <returns></returns>
char* UI_MakeBindingGetterFunctionName(const char* inBindingName);

char* UI_MakeBindingSetterFunctionName(const char* inBindingName);


#endif
