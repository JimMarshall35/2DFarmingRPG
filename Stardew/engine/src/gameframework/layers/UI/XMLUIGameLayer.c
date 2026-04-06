#include "XMLUIGameLayer.h"
#include "DrawContext.h"
#include "InputContext.h"
#include "GameFramework.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "IntTypes.h"
#include "DynArray.h"
#include "ObjectPool.h"
#include "Widget.h"
#include "Widget.h"
#include "StaticWidget.h"
#include "StackPanelWidget.h"
#include "Atlas.h"
#include "RootWidget.h"
#include "main.h"
#include "TextWidget.h"
#include "BackgroundBoxWidget.h"
#include "Scripting.h"
#include "Geometry.h"
#include "AssertLib.h"
#include "TextButtonWidget.h"
#include "RadioButtonWidget.h"
#include "RadioGroupWidget.h"
#include "SliderWidget.h"
#include "CanvasWidget.h"
#include "TextEntryWidget.h"
#include "DataNode.h"
#include "StringKeyHashMap.h"
#include "GameFrameworkEvent.h"
#include <libxml/parser.h>
#include <libxml/tree.h>
#include "Log.h"
#include "cwalk.h"
#include "main.h"
#include "XMLHelpers.h"

static struct HashMap gNodeNameMap;
static bool gInitialisedNodeNameMap = false;


static void InitializeNodeNameMap()
{
	HashmapInit(&gNodeNameMap, 20, sizeof(AddChildFn));

	AddChildFn fn = &StackPanelWidgetNew;
	HashmapInsert(&gNodeNameMap, "stackpanel", &fn);
	fn = &StaticWidgetNew;
	HashmapInsert(&gNodeNameMap, "static", &fn);
	fn = &TextWidgetNew;
	HashmapInsert(&gNodeNameMap, "text", &fn);
	fn = &BackgroundBoxWidgetNew;
	HashmapInsert(&gNodeNameMap, "backgroundbox", &fn);
	fn = &TextButtonWidgetNew;
	HashmapInsert(&gNodeNameMap, "textButton", &fn);
	fn = &RadioGroupWidgetNew;
	HashmapInsert(&gNodeNameMap, "radioGroup", &fn);
	fn = &RadioButtonWidgetNew;
	HashmapInsert(&gNodeNameMap, "radioButton", &fn);
	fn = &SliderWidgetNew;
	HashmapInsert(&gNodeNameMap, "slider", &fn);
	fn = &CanvasWidgetNew;
	HashmapInsert(&gNodeNameMap, "canvas", &fn);
	fn = &TextEntryWidgetNew;
	HashmapInsert(&gNodeNameMap, "textInput", &fn);
}


AddChildFn LookupWidgetCtor(const char* widgetName)
{
	return *((AddChildFn*)HashmapSearch(&gNodeNameMap, widgetName));
}

static void FreeWidgetTree_internal(HWidget root, bool bFreeRoot)
{
	struct UIWidget* pWidget = UI_GetWidget(root);
	struct UIWidget* pWidgetRoot = pWidget;
	if (!pWidget)
	{
		return;
	}

	HWidget h = pWidget->hFirstChild;
	while (h != NULL_HWIDGET)
	{
		HWidget oldH = h;
		pWidget = UI_GetWidget(h);

		h = pWidget->hNext;
		FreeWidgetTree_internal(oldH, true);
	}
	pWidgetRoot->hFirstChild = NULL_HWIDGET;
	if (bFreeRoot)
	{
		UI_DestroyWidget(root);
	}

}

static void FreeWidgetTree(HWidget root)
{
	FreeWidgetTree_internal(root, true);
}

static void FreeWidgetChildren(HWidget root)
{
	FreeWidgetTree_internal(root, false);
}

/*
	add the TOS lua table as a child of hParent, and recurse throught children
*/
static void AddLuaTableSubTree(XMLUIData* pData, HWidget hParent)
{
	/*
		Add the lua node and its children here:
	*/
	struct DataNode node;
	DN_InitForLuaTableOnTopOfStack(&node);
	Sc_TableGet("type");
	size_t len = Sc_StackTopStringLen();
	if (len)
	{
		char* pStr = malloc(len + 1);
		Sc_StackTopStrCopy(pStr);
		Sc_Pop();

		AddChildFn fn = LookupWidgetCtor(pStr);
		HWidget hNew = fn(hParent, &node, pData);
		free(pStr);

		struct UIWidget* pWiddget = UI_GetWidget(hNew);
		UI_WidgetCommonInit(&node, pWiddget);
		pWiddget->hNext = NULL_HANDLE;
		pWiddget->scriptCallbacks.viewmodelTable = pData->hViewModel;

		UI_AddChild(hParent, hNew);
		Sc_TableGet("children");
		for (int i = 1; i <= Sc_TableLen(); i++)
		{
			Sc_TableGetIndex(i);
			if (Sc_IsTable())
			{
				AddLuaTableSubTree(pData, hNew);
			}
			Sc_Pop();
		}
		Sc_Pop();
	}
	else
	{
		Log_Error("error: child node in children table is not a string or is empty");
		Sc_Pop();
	}
}

static void Update(struct GameFrameworkLayer* pLayer, float deltaT)
{
	XMLUIData* pData = pLayer->userData;
	TP_DoTimers(&pData->timerPool, deltaT);
	if (VectorSize(pData->pChildrenChangeRequests))
	{
		struct WidgetChildrenChangeRequest* pReq = &pData->pChildrenChangeRequests[0];
		FreeWidgetChildren(pReq->hWidget);
		Sc_CallFuncInRegTableEntryTable(pReq->regIndex, pReq->funcName, NULL, 0, 1);
		int t = Sc_Type();
		if (Sc_IsTable())
		{
			for (int i = 1; i <= Sc_TableLen(); i++)
			{
				Sc_TableGetIndex(i);

				if (Sc_IsTable())
				{
					AddLuaTableSubTree(pData, pReq->hWidget);
				}

				Sc_Pop();
			}
			Sc_Pop();
			struct UIWidget* pWidget = UI_GetWidget(pReq->hWidget);
			if (pWidget)
			{
				if (pWidget->fnOnWidgetChildrenChangedFn)
				{
					pWidget->fnOnWidgetChildrenChangedFn(pWidget);
				}
			}
			SetRootWidgetIsDirty(pData->rootWidget, true);
		}
		else
		{
			Log_Error("error: function %s did not return a table", pReq->funcName);
		}
	}
	for (int i = 0; i < VectorSize(pData->pChildrenChangeRequests); i++)
	{
		free(pData->pChildrenChangeRequests[i].funcName);
	}
	pData->pChildrenChangeRequests = VectorClear(pData->pChildrenChangeRequests);
	Sc_ResetStack();
}

static void UpdateRootWidget(XMLUIData* pData, DrawContext* dc)
{
	VectorClear(pData->pWidgetVertices);
	struct UIWidget* pRootWidget = UI_GetWidget(pData->rootWidget);
	pRootWidget->fnLayoutChildren(pRootWidget, NULL);
	pData->pWidgetVertices = pRootWidget->fnOutputVertices(pRootWidget, pData->pWidgetVertices);
	dc->UIVertexBufferData(pData->hVertexBuffer, pData->pWidgetVertices, VectorSize(pData->pWidgetVertices));
	SetRootWidgetIsDirty(pData->rootWidget, false);
}

static void Draw(struct GameFrameworkLayer* pLayer, DrawContext* dc)
{
	XMLUIData* pData = pLayer->userData;
	At_SetCurrent(pData->atlas, dc);
	struct UIWidget* pRootWidget = UI_GetWidget(pData->rootWidget);
	if (!pRootWidget)
	{
		Log_Error("something wrong");
		return;
	}

	if (GetRootWidgetIsDirty(pData->rootWidget))
	{
		UpdateRootWidget(pData, dc);
	}
	int size = VectorSize(pData->pWidgetVertices);

	dc->DrawUIVertexBuffer(pData->hVertexBuffer, size);
}


static void* BuildWidgetsHoverred(VECTOR(HWidget) outWidgets, HWidget hWidget, float mouseX, float mouseY)
{
	GeomRect hitbox;
	UI_GetHitBox(hitbox, hWidget);
	if (Ge_PointInAABB(mouseX, mouseY, hitbox))
	{
		outWidgets = VectorPush(outWidgets, &hWidget);

		struct UIWidget* pWidget = UI_GetWidget(hWidget);
		HWidget hChild = pWidget->hFirstChild;
		while (hChild != NULL_HWIDGET)
		{
			outWidgets = BuildWidgetsHoverred(outWidgets, hChild, mouseX, mouseY);
			struct UIWidget* pChild = UI_GetWidget(hChild);
			hChild = pChild->hNext;
		}
	}
	return outWidgets;

}

static void Unfocus(XMLUIData* pUIData)
{
	for (int i = 0; i < pUIData->nFocusedWidgets; i++)
	{
		struct UIWidget* pWidget = UI_GetWidget(pUIData->focusedWidgets[i]);
		EASSERT(pWidget->bAcceptsFocus);
		pWidget->bHasFocus = false;
		if (pWidget->cCallbacks.Callbacks[WC_OnLeaveFocus].callback.bActive)
		{
			pWidget->cCallbacks.Callbacks[WC_OnLeaveFocus].callback.focusChangeFn(pWidget);
		}
	}
	pUIData->nFocusedWidgets = 0;
}

static bool SendMouseDownAndFocus(struct WidgetMouseInfo* info, HWidget hWidget, XMLUIData* pUIData)
{
	Unfocus(pUIData);
	struct UIWidget* pWidget = UI_GetWidget(hWidget);
	bool rval = false;
	if (pWidget->bAcceptsFocus)
	{
		rval = true;
		pWidget->bHasFocus = true;
		pUIData->focusedWidgets[pUIData->nFocusedWidgets++] = hWidget;
		if (pWidget->cCallbacks.Callbacks[WC_OnGainFocus].callback.bActive)
		{
			pWidget->cCallbacks.Callbacks[WC_OnGainFocus].callback.focusChangeFn(pWidget);
		}
	}
	UI_SendWidgetMouseEvent(pWidget, WC_OnMouseDown, info);
	return rval;
}

static void Input(struct GameFrameworkLayer* pLayer, InputContext* ctx)
{
	static VECTOR(HWidget) pWidgetsHovverred = NULL;
	static VECTOR(HWidget) pWidgetsHovverredLastFrame = NULL;
	static VECTOR(HWidget) pWidgetsEntered = NULL;
	static VECTOR(HWidget) pWidgetsLeft = NULL;
	static VECTOR(HWidget) pWidgetsRemained = NULL;

	static bool bLastLeftClick = false;
	static bool bThisLeftClick = false;

	if (!pWidgetsHovverred)
	{
		pWidgetsHovverred = NEW_VECTOR(HWidget);
	}
	if (!pWidgetsHovverredLastFrame)
	{
		pWidgetsHovverredLastFrame = NEW_VECTOR(HWidget);
	}
	if (!pWidgetsEntered)
	{
		pWidgetsEntered = NEW_VECTOR(HWidget);
	}
	if (!pWidgetsLeft)
	{
		pWidgetsLeft = NEW_VECTOR(HWidget);
	}
	if (!pWidgetsRemained)
	{
		pWidgetsRemained = NEW_VECTOR(HWidget);
	}

	pWidgetsHovverredLastFrame = VectorClear(pWidgetsHovverredLastFrame);
	for (int i = 0; i < VectorSize(pWidgetsHovverred); i++)
	{
		pWidgetsHovverredLastFrame = VectorPush(pWidgetsHovverredLastFrame, &pWidgetsHovverred[i]);
	}

	pWidgetsHovverred = VectorClear(pWidgetsHovverred);
	pWidgetsEntered = VectorClear(pWidgetsEntered);
	pWidgetsLeft = VectorClear(pWidgetsLeft);
	pWidgetsRemained = VectorClear(pWidgetsRemained);

	XMLUIData* pUIData = pLayer->userData;
	//pUIData->pChildrenChangeRequests = VectorClear(pUIData->pChildrenChangeRequests);
	float w, h;

	bThisLeftClick = In_GetButtonValue(ctx, pUIData->gMouseBtnLeft);

	vec2 mousePos = { In_GetAxisValue(ctx, pUIData->gMouseX), In_GetAxisValue(ctx, pUIData->gMouseY) };


	pWidgetsHovverred = BuildWidgetsHoverred(pWidgetsHovverred, pUIData->rootWidget, mousePos[0], mousePos[1]);

	// any widgets started to be hoverred?
	for (int i = 0; i < VectorSize(pWidgetsHovverred); i++)
	{
		HWidget hHovvered = pWidgetsHovverred[i];
		bool bWasHoverredLastFrame = false;
		for (int j = 0; j < VectorSize(pWidgetsHovverredLastFrame); j++)
		{
			if (hHovvered == pWidgetsHovverredLastFrame[j])
			{
				bWasHoverredLastFrame = true;
				break;
			}
		}
		if (!bWasHoverredLastFrame)
		{
			pWidgetsEntered = VectorPush(pWidgetsEntered, &hHovvered);
		}
		else
		{
			pWidgetsRemained = VectorPush(pWidgetsRemained, &hHovvered);
		}
	}

	// any widgets stopped being hovvered?
	for (int i = 0; i < VectorSize(pWidgetsHovverredLastFrame); i++)
	{
		HWidget hHovvered = pWidgetsHovverredLastFrame[i];
		bool bHoverredThisFrame = false;
		for (int j = 0; j < VectorSize(pWidgetsHovverred); j++)
		{
			if (hHovvered == pWidgetsHovverred[j])
			{
				bHoverredThisFrame = true;
				break;;
			}
		}
		if (!bHoverredThisFrame)
		{
			pWidgetsLeft = VectorPush(pWidgetsLeft, &hHovvered);
		}
	}

	struct WidgetMouseInfo info =
	{
		.x = mousePos[0],
		.y = mousePos[1],
		.button = 0
	};

	bool bSendLMouseDown = false;
	bool bSendLMouseUp = false;
	if (bThisLeftClick && !bLastLeftClick)
	{
		info.button = 0;
		bSendLMouseDown = true;
	}
	else if (!bThisLeftClick && bLastLeftClick)
	{
		info.button = 0;
		bSendLMouseUp = true;
	}

	bLastLeftClick = bThisLeftClick;


	for (int i = 0; i < VectorSize(pWidgetsEntered); i++)
	{
		HWidget hWidget = pWidgetsEntered[i];
		struct UIWidget* pWidget = UI_GetWidget(hWidget);
		UI_SendWidgetMouseEvent(pWidget, WC_OnMouseEnter, &info);
	}
	for (int i = 0; i < VectorSize(pWidgetsLeft); i++)
	{
		HWidget hWidget = pWidgetsLeft[i];
		struct UIWidget* pWidget = UI_GetWidget(hWidget);
		UI_SendWidgetMouseEvent(pWidget, WC_OnMouseLeave, &info);
	}


	bool bFocusChanged = false;
	bool bWidgetsClicked = VectorSize(pWidgetsRemained) > 0 && bSendLMouseDown;


	for (int i = 0; i < VectorSize(pWidgetsRemained); i++)
	{
		HWidget hWidget = pWidgetsRemained[i];
		struct UIWidget* pWidget = UI_GetWidget(hWidget);
		if (bSendLMouseDown)
		{
			EASSERT(!bSendLMouseUp);
			if (SendMouseDownAndFocus(&info, hWidget, pUIData))
			{
				bFocusChanged = true;
			}
		}
		if (bSendLMouseUp)
		{
			EASSERT(!bSendLMouseDown);
			UI_SendWidgetMouseEvent(pWidget, WC_OnMouseUp, &info);
		}
		UI_SendWidgetMouseEvent(pWidget, WC_OnMouseMove, &info);
	}

	for (int i = 0; i < pUIData->nFocusedWidgets; i++)
	{
		HWidget hWidget = pUIData->focusedWidgets[i];
		struct UIWidget* pWidget = UI_GetWidget(hWidget);
		for (int j = 0; j < ctx->textInput.nKeystrokesThisFrame; j++)
		{
			int keystroke = ctx->textInput.keystrokes[j];
			if (pWidget->fnRecieveKeystroke)
				pWidget->fnRecieveKeystroke(pWidget, keystroke);
		}
	}

	if (Sc_FunctionPresentInTable(pUIData->hViewModel, "OnInput"))
	{
		Sc_CallFuncInRegTableEntryTable(pUIData->hViewModel, "OnInput", NULL, 0, 0);
	}

}

static void LoadUIData(XMLUIData* pUIData, DrawContext* pDC);


static void OnPush(struct GameFrameworkLayer* pLayer, DrawContext* drawContext, InputContext* inputContext)
{
	XMLUIData* pData = pLayer->userData;
	pData->timerPool = TP_InitTimerPool(32);

	if (!pData->bLoaded)
	{
		LoadUIData(pData, drawContext);
	}
	hTexture hAtlasTex = At_GetAtlasTexture(pData->atlas);
	drawContext->SetCurrentAtlas(hAtlasTex);
	if (Sc_FunctionPresentInTable(pData->hViewModel, "OnXMLUILayerPush"))
	{
		Sc_CallFuncInRegTableEntryTable(pData->hViewModel, "OnXMLUILayerPush", NULL, 0, 0);
	}

	struct ActiveInputBindingsMask mask;
	In_GetMask(&mask, inputContext);
	pData->gMouseBtnLeft = In_FindButtonMapping(inputContext, "select");
	pData->gMouseY = In_FindAxisMapping(inputContext, "CursorPosY");
	pData->gMouseX = In_FindAxisMapping(inputContext, "CursorPosX");
	In_ActivateButtonBinding(pData->gMouseBtnLeft, &mask);
	In_ActivateAxisBinding(pData->gMouseY, &mask);
	In_ActivateAxisBinding(pData->gMouseX, &mask); /* TODO: this is incomplete: these */
	In_SetMask(&mask, inputContext);
}


static void OnPop(struct GameFrameworkLayer* pLayer, DrawContext* drawContext, InputContext* inputContext)
{
	XMLUIData* pData = pLayer->userData;
	if (Sc_FunctionPresentInTable(pData->hViewModel, "OnXMLUILayerPop"))
	{
		Sc_CallFuncInRegTableEntryTable(pData->hViewModel, "OnXMLUILayerPop", NULL, 0, 0);
	}
	DestoryVector(pData->pWidgetVertices);
	drawContext->DestroyVertexBuffer(pData->hVertexBuffer);
	FreeWidgetTree(pData->rootWidget);
	if (pData->hViewModel)
	{
		Sc_DeleteTableInReg(pData->hViewModel);
	}
	TP_DestroyTimerPool(&pData->timerPool);

	struct ActiveInputBindingsMask mask;
	In_GetMask(&mask, inputContext);
	In_DeactivateButtonBinding(pData->gMouseBtnLeft, &mask);
	In_DeactivateAxisBinding(pData->gMouseX, &mask);
	In_DeactivateAxisBinding(pData->gMouseY, &mask);
	In_SetMask(&mask, inputContext);
}

void AddNodeChildren(HWidget widget, xmlNode* pNode, XMLUIData* pUIData)
{
	struct DataNode dataNode;
	for (xmlNode* pChild = pNode->children; pChild; pChild = pChild->next)
	{
		DN_InitForXMLNode(&dataNode, pChild);
		if (pChild->type != XML_ELEMENT_NODE)
		{
			continue;
		}
		AddChildFn pCtor = LookupWidgetCtor(pChild->name);
		if (!pCtor)
		{
			// log error
			Log_Error("error occured");
			return;
		}
		HWidget childWidget = pCtor(widget, &dataNode, pUIData);

		struct UIWidget* pWiddget = UI_GetWidget(childWidget);
		pWiddget->scriptCallbacks.viewmodelTable = pUIData->hViewModel;
		UI_WidgetCommonInit(&dataNode, pWiddget);

		UI_AddChild(widget, childWidget);

		AddNodeChildren(childWidget, pChild, pUIData);
	}
}



void LoadAtlas(XMLUIData* pUIData, xmlNode* child0, DrawContext* pDC)
{
	pUIData->atlas = At_LoadAtlas(child0, pDC);
}

static bool TryLoadViewModel(XMLUIData* pUIData, xmlNode* pScreenNode)
{
	bool rVal = false;
	bool bVMFileSet = false;
	bool bVMFunctionSet = false;
	char* pFilePath = NULL;
	char* pFnName = NULL;
	xmlChar* attribute = NULL;
	if (attribute = xmlGetProp(pScreenNode, "viewmodelFile"))
	{
		pFilePath = attribute;
		bVMFileSet = true;
	}
	if (attribute = xmlGetProp(pScreenNode, "viewmodelFunction"))
	{
		pFnName = attribute;
		bVMFunctionSet = true;
	}

	if (bVMFileSet && bVMFunctionSet)
	{
		char buf[256];
		cwk_path_join(gCmdArgs.assetsDir, pFilePath, buf, 256);
		Log_Verbose("opening viewmodel file %s", buf);
		// instantiate viewmodel lua object and store in registry
		Sc_OpenFile(buf);
		Log_Verbose("done");
		pUIData->hViewModel = Sc_CallGlobalFuncReturningTableAndStoreResultInReg(pFnName, NULL, 0);

		// tag the viewmodel table with a ptr to the XMLUIDataPtr so it can set the widget tree flag to dirty
		Sc_AddLightUserDataValueToTable(pUIData->hViewModel, "XMLUIDataPtr", pUIData);
	}
	else
	{
		Log_Error("TryLoadViewModel, either file or function (or both) not set. file: %i function name: %i", bVMFileSet, bVMFunctionSet);
	}

	if (pFnName)
	{
		xmlFree(pFnName);
	}
	if (pFilePath)
	{
		xmlFree(pFilePath);
	}

	return rVal;
}

static void InitializeWidgets(HWidget root)
{
	struct UIWidget* pWidget = UI_GetWidget(root);
	if (pWidget->fnOnWidgetInit)
	{
		pWidget->fnOnWidgetInit(pWidget);
	}
	HWidget child = pWidget->hFirstChild;
	while (child != NULL_HWIDGET)
	{
		InitializeWidgets(child);
		struct UIWidget* pWidget = UI_GetWidget(child);
		child = pWidget->hNext;
	}
}

static void LoadUIData(XMLUIData* pUIData, DrawContext* pDC)
{
	assert(!pUIData->bLoaded);
	pUIData->bLoaded = true;
	xmlDoc* pXMLDoc = xmlReadFile(pUIData->xmlFilePath, NULL, 0);

	if (!gInitialisedNodeNameMap)
	{
		gInitialisedNodeNameMap = true;
		InitializeNodeNameMap();
	}

	char nodeNameArr[128];

	if (pXMLDoc)
	{
		Log_Verbose("pXMLDoc is valid");
		xmlNode* root = xmlDocGetRootElement(pXMLDoc);
		unsigned long numchildren = xmlChildElementCount(root);
		if (numchildren != 2)
		{
			Log_Error("%s root should have 2 kids", __FUNCTION__);
			xmlFreeDoc(pXMLDoc);
			return;
		}
		xmlNode* child0 = GetNthChild(root, 0);
		xmlNode* child1 = GetNthChild(root, 1);

		bool bDoneAtlas = false;
		bool bDoneScreen = false;

		if (strcmp(child0->name, "atlas") == 0)
		{
			bDoneAtlas = true;
			LoadAtlas(pUIData, child0, pDC);
		}
		else if (strcmp(child0->name, "screen") == 0)
		{
			bDoneScreen = true;
			TryLoadViewModel(pUIData, child0);
			AddNodeChildren(pUIData->rootWidget, child0, pUIData);
		}

		if (strcmp(child1->name, "atlas") == 0 && !bDoneAtlas)
		{
			bDoneAtlas = true;
			LoadAtlas(pUIData, child1, pDC);
		}
		else if (strcmp(child1->name, "screen") == 0 && !bDoneScreen)
		{
			bDoneScreen = true;
			TryLoadViewModel(pUIData, child1);
			AddNodeChildren(pUIData->rootWidget, child1, pUIData);
		}
		if (!bDoneAtlas || !bDoneScreen)
		{
			Log_Error("%s ui xml file doesn't have both screen and atlas components", __FUNCTION__);
		}

		xmlFreeDoc(pXMLDoc);

		struct UIWidget* pWidget = UI_GetWidget(pUIData->rootWidget);
		pWidget->scriptCallbacks.viewmodelTable = pUIData->hViewModel;

		InitializeWidgets(pUIData->rootWidget);

		pUIData->hVertexBuffer = pDC->NewUIVertexBuffer(2048);
	}

}

static void OnWindowSizeChanged(struct GameFrameworkLayer* pLayer, int newW, int newH)
{
	XMLUIData* pData = pLayer->userData;
	RootWidget_OnWindowSizeChanged(pData->rootWidget, newW, newH);

	struct UIWidget* pWidget = UI_GetWidget(pData->rootWidget);
	SetRootWidgetIsDirty(pData->rootWidget, true);
}


void XMLUIGameLayer_Get(struct GameFrameworkLayer* pLayer, struct XMLUIGameLayerOptions* pOptions)
{
	pLayer->userData = malloc(sizeof(XMLUIData));
	if (!pLayer->userData) { Log_Error("XMLUIGameLayer_Get: no memory"); return; }
	XMLUIData* pUIData = (XMLUIData*)pLayer->userData;

	memset(pLayer->userData, 0, sizeof(XMLUIData));

	strcpy(pUIData->xmlFilePath, pOptions->xmlPath);
	pUIData->rootWidget = NewRootWidget();
	RootWidget_OnWindowSizeChanged(pUIData->rootWidget, Mn_GetScreenWidth(), Mn_GetScreenHeight());
	pLayer->draw = &Draw;
	pLayer->update = &Update;
	pLayer->input = &Input;
	pLayer->onPop = &OnPop;
	pLayer->onPush = &OnPush;
	pLayer->onWindowDimsChanged = &OnWindowSizeChanged;
	pLayer->flags = 0;
	pLayer->flags |= EnableDrawFn | EnableInputFn | EnableUpdateFn | EnableOnPop | EnableOnPush;
	pUIData->pWidgetVertices = NEW_VECTOR(WidgetVertex);
	pUIData->pChildrenChangeRequests = NEW_VECTOR(struct WidgetChildrenChangeRequest);
	if (pOptions->bLoadImmediately)
	{
		LoadUIData(pUIData, pOptions->pDc);
	}

}


void XMLUI_PushGameFrameworkLayer(const char* xmlPath)
{
	struct GameFrameworkLayer testLayer;
	memset(&testLayer, 0, sizeof(struct GameFrameworkLayer));
	struct XMLUIGameLayerOptions options;
	options.bLoadImmediately = false;
	options.xmlPath = xmlPath;
	options.pDc = NULL;
	testLayer.flags |= (EnableOnPush | EnableOnPop);
	Log_Verbose("making xml ui layer");
	XMLUIGameLayer_Get(&testLayer, &options);
	Log_Verbose("done");
	Log_Verbose("pushing framework layer");
	GF_PushGameFrameworkLayer(&testLayer);
}