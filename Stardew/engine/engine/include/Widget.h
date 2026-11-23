#ifndef WIDGET_H
#define WIDGET_H
#ifdef __cplusplus
extern "C" {
#endif

#include "ObjectPool.h"
#include "HandleDefs.h"
#include <stdbool.h>
#include "DynArray.h"
#include "Geometry.h"


struct UIWidget;
struct WidgetPropertyBinding;
struct DataNode;



#ifdef  __cplusplus
typedef int(*PrintfFn)(const char* fmt, ...);

#else
typedef int(*PrintfFn)(const char* restrict fmt, ...);
#endif //  __cplusplus

struct Vert2DColourTexture;
typedef struct Vert2DColourTexture WidgetVertex;
struct Vert2DColourTextureQuad;
typedef struct Vert2DColourTextureQuad WidgetQuad;

typedef float(*GetUIWidgetDimensionFn)(struct UIWidget* pWidget, struct UIWidget* pParent);
typedef void(*LayoutChildrenFn)(struct UIWidget* pWidget, struct UIWidget* pParent);
typedef void(*OnDestroyWidgetFn)(struct UIWidget* pWidget);
typedef void*(*OutputWidgetVerticesFn)(struct UIWidget* pThisWidget, VECTOR(WidgetVertex) pOutVerts);
typedef void(*OnWidgetInitFn)(struct UIWidget* pWidget);
typedef void(*OnBoundPropertyChangedFn)(struct UIWidget* pThisWidget, struct WidgetPropertyBinding* pBinding);

typedef struct WidgetDim* (*WidgetDimGetterFn)(struct UIWidget* pWidget);

typedef void (*FocusedWidgetRecieveKeystrokeFn)(struct UIWidget* pWidget, int keystroke);

typedef void (*OnChildrenChangedFn)(struct UIWidget* pWidget);

struct WidgetDim* GetWidgetWidthDim(struct UIWidget* pWidget);
struct WidgetDim* GetWidgetHeightDim(struct UIWidget* pWidget);

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

/// @brief 	some are relative to their parent. In which case these should be relative to THEIR parent, fixed, or the top level window.
enum WidgetDimType
{
	/// @brief take as much space as it naturally takes
	WD_Auto,
	/// @brief stretch to fill all available space
	WD_Stretch,
	/// @brief a literal value
	WD_Pixels,
	
	/// @brief a fraction that's relative with other children.
	///	meaning this:
	/// width: 1* ------|
	/// width: 2* -----------|
	///               | a |   b  |
	/// ui element a is twice the width of ui element b, and together they take up the entirety of their parents width
	WD_StretchFraction,
	/// @brief a percentage of parents dimension but as a decimal between 0 and 1 
	WD_Percentage
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
	WC_OnGainFocus,
	WC_OnLeaveFocus,
	WC_NUM
};

typedef void (*WidgetMousePosCallbackFn)(struct UIWidget* pWidget, float x, float y);
typedef void (*WidgetMouseBtnCallbackFn)(struct UIWidget* pWidget, float x, float y, int btn);
typedef void (*WidgetFocusChangeCallbackFn)(struct UIWidget* pWidget);

/// @brief this isn't some "hungarian notation" shit - the name refers to widget callbacks defined as C functions!
struct CWidgetMouseCallback
{
	enum WidgetCallbackTypes type;
	union
	{
		WidgetMousePosCallbackFn mousePosFn;
		WidgetMouseBtnCallbackFn mouseBtnFn;
		WidgetFocusChangeCallbackFn focusChangeFn;

		/// @brief A random value we can check is zero to determine if the callback is set
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
	WBT_WidgetTree
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
	HWidget hThis;
	void* pImplementationData;
	GetUIWidgetDimensionFn fnGetWidth;
	GetUIWidgetDimensionFn fnGetHeight;
	LayoutChildrenFn fnLayoutChildren;
	OnDestroyWidgetFn fnOnDestroy;
	OutputWidgetVerticesFn fnOutputVertices;
	OnBoundPropertyChangedFn fnOnBoundPropertyChanged;
	OnWidgetInitFn fnOnWidgetInit; // called once whole widget tree is constructed
	FocusedWidgetRecieveKeystrokeFn fnRecieveKeystroke;
	OnChildrenChangedFn fnOnWidgetChildrenChangedFn;
	float top;
	float left;
	WidgetDockPoint dockPoint;
	struct WidgetPadding padding;
	struct LuaWidgetCallbacks scriptCallbacks;
	struct CWidgetCallbacks cCallbacks;
	struct WidgetPropertyBinding bindings[MAX_NUM_BINDINGS];
	struct WidgetDim width;
	struct WidgetDim height;
	size_t numBindings;
	float offsetX;
	float offsetY;
	WidgetHorizontalAlignment horizontalAlignment;
	WidgetVerticalAlignment verticalAlignment;
	bool bAcceptsFocus;
	bool bHasFocus;
};


void UI_Init();


void UI_DestroyWidget(HWidget widget);

HWidget UI_GetScratchWiget();

HWidget UI_NewBlankWidget();

size_t UI_CountWidgetChildrenPtr(struct UIWidget* pWidget);

size_t UI_CountWidgetChildren(HWidget pWidget);

struct UIWidget* UI_GetWidget(HWidget hWidget);

struct UIWidget* UI_FirstChild(HWidget hWidget);

void UI_AddChild(HWidget hParent, HWidget hChild);

float UI_ResolveWidthDimPxls(struct UIWidget* pWidget, const struct WidgetDim* dim);

float UI_ResolveHeightDimPxls(struct UIWidget* pWidget, const struct WidgetDim* dim);


/// @brief Init common attributes that all widgets can have
/// - "the base class constructor"
/// @param pInNode 
/// @param outWidget 
void UI_WidgetCommonInit(struct DataNode* pInNode, struct UIWidget* outWidget);

/// @brief print a text representation of the widget tree.
/// Falls appart if a widget fails to implement its debug print hook correctly.
/// Shit. DON'T USE
/// @param inWidget 
/// @param pPrintfFn 
void UI_DebugPrintCommonWidgetInfo(const struct UIWidget* inWidget, PrintfFn pPrintfFn);

void* UI_Helper_OnOutputVerts(struct UIWidget* pWidget, VECTOR(WidgetVertex) pOutVerts);

void UI_Helper_OnLayoutChildren(struct UIWidget* pWidget, struct UIWidget* pParent);

void UI_GetWidgetSize(HWidget hWidget, float* pOutW, float* pOutH);

/// @brief get top left BEFORE padding is applied (the raw top left)
/// @param hWidget 
/// @param pOutLeft 
/// @param pOutTop 
void UI_GetWidgetTopLeft(HWidget hWidget, float* pOutLeft, float* pOutTop);

void UI_GetWidgetPadding(HWidget hWidget, float* pOutPaddingTop, float* pOutPaddingBottom, float* pOutPaddingLeft, float* pOutPaddingRight);

/// @brief send a mouse event to the widget that will be handled either by either or both a lua function and a 
/// c function. If both the C function is called first
/// @param pWidget 
/// @param  
/// @param pMouseInfo data about the event
void UI_SendWidgetMouseEvent(struct UIWidget* pWidget, enum WidgetCallbackTypes type, struct WidgetMouseInfo* pMouseInfo);

/// @brief Get the AABB of the widget, excluding padding.
/// @param outRect 
/// @param hWidget 
void UI_GetHitBox(GeomRect outRect, HWidget hWidget);

/// @brief 
/// @param attributeValue 
/// @return is the value of an xml attribute property
/// an absolute value (return false) or is it deferrred to
/// a getter on the lua viewmodel (return true).
/// Special syntax for this is to surround the name of a view model property with "{" and "}"
bool UI_IsAttributeStringABindingExpression(const char* attributeValue);

/// @brief Add a bound string property entry to the widget
/// and initialise a value by calling the binding getter function
/// @param pWidget theWidget to add the property binding. When the value changes 
///		the property of the widget will change when OnPropertyChanged is called.
///     2 way bindings will also be introduced for sliders, text entry fields, ect
/// @param inBoundPropertyName bound property name - as it appears in the viewmodel lua table
/// @param inBindingExpression the incoming binding expression, i.e. {spritename}
/// @param pOutData the value returned from calling the lua string getter - allocates a string setting the pointer.
///		user must deallocate
/// @param viewmodelTableIndex the index in the lua registry that contains the lua table containing the function to call
void UI_AddStringPropertyBinding(struct UIWidget* pWidget, char* inBoundPropertyName, char* inBindingExpression, char** pOutData, int viewmodelTableIndex);

void UI_AddIntPropertyBinding(struct UIWidget* pWidget, char* inBoundPropertyName, char* inBindingExpression, int* pOutData, int viewmodelTableIndex);

void UI_AddFloatPropertyBinding(struct UIWidget* pWidget, char* inBoundPropertyName, char* inBindingExpression, float* pOutData, int viewmodelTableIndex);

/// @brief USER MUST FREE RETURNED PTR
/// append Get_ to the start of the string.
/// @param inBindingName 
/// @return 
char* UI_MakeBindingGetterFunctionName(const char* inBindingName);

char* UI_MakeBindingSetterFunctionName(const char* inBindingName);

struct WidgetPropertyBinding* UI_FindBinding(struct UIWidget* pWidget, const char* bindingName);

void UI_DefaultOnChildrenChanged(struct UIWidget* outWidget);

#ifdef __cplusplus
}
#endif

#endif
