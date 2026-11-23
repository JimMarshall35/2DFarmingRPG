#include "TextEntryWidget.h"
#include "Widget.h"
#include "DataNode.h"
#include "XMLUIGameLayer.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "TextWidget.h"
#include "Atlas.h"
#include "AssertLib.h"
#include "CanvasWidget.h"
#include "InputContext.h"
#include "RootWidget.h"
#include "TimerPool.h"
#include "Scripting.h"
#include "Log.h"

struct TextEntryWidgetData
{
	struct TextWidgetData textWidget;
	struct CanvasData canvasWidget;
	// uses the TextWidgetData content pointer directly to store
	// the string as it's being manipulated
	int maxStringLen;
	int currentStringLen;
	int cursorIndex;
	XMLUIData* pLayerData;
	char caretChar;
	HTimer caretTimer;
	bool bCaretBlinkState;
	char onEnterPressLuaCallbackName[TEXT_WIDGET_DATA_LUA_CALLBACK_NAME_BUFFER_SIZE];
	bool bEnterPressCallbackSet;
	int viewmodelRegIndex;
};

static float GetWidth(struct UIWidget* pWidget, struct UIWidget* pParent)
{
	return UI_ResolveWidthDimPxls(pWidget, &pWidget->width) + pWidget->padding.paddingLeft + pWidget->padding.paddingRight;
}

static float GetHeight(struct UIWidget* pWidget, struct UIWidget* pParent)
{
	struct TextEntryWidgetData* pData = (struct TextEntryWidgetData*)pWidget->pImplementationData;
	return pData->textWidget.fSizePts * 1.33333333 + pWidget->padding.paddingTop + pWidget->padding.paddingBottom;
}

static void LayoutChildren(struct UIWidget* pWidget, struct UIWidget* pParent)
{
}

static void OnDestroy(struct UIWidget* pWidget)
{
	TextWidget_Destroy(&((struct TextEntryWidgetData*)&pWidget->pImplementationData)->textWidget);
	//free(pWidget->pImplementationData);
}

static void* FakeChildOutputVerts(struct UIWidget* pWidget, VECTOR(WidgetVertex) pOutVerts)
{
	return TextWidget_OutputVerts(pWidget->left, pWidget->top, &pWidget->padding, pWidget->pImplementationData, pOutVerts);
}

static void* OnOutputVerts(struct UIWidget* pWidget, VECTOR(WidgetVertex) pOutVerts)
{
	struct TextEntryWidgetData* pData = pWidget->pImplementationData;
	pData->canvasWidget.bHSliderActive = false;
	pData->canvasWidget.bVSliderActive = false;

	HWidget hFake = UI_GetScratchWiget();
	struct UIWidget* fakeChild = UI_GetWidget(hFake);

	memcpy(fakeChild, pWidget, sizeof(struct UIWidget));
	fakeChild->hNext = NULL_HANDLE;
	fakeChild->fnOutputVertices = &FakeChildOutputVerts;
	fakeChild->top += pWidget->padding.paddingTop;
	fakeChild->left += pWidget->padding.paddingLeft;
	fakeChild->dockPoint = WDP_BottomLeft;
	memset(&fakeChild->padding, 0, sizeof(struct WidgetPadding));
	fakeChild->pImplementationData = &pData->textWidget;
	pWidget->hFirstChild = hFake;
	pOutVerts = CanvasWidget_OnOutputVerts(pWidget, pOutVerts);
	pWidget->hFirstChild = NULL_HANDLE;
	
	if(pData->bCaretBlinkState)
		pOutVerts = TextWidget_OutputAtLetter(pWidget->left, pWidget->top, &pWidget->padding, &pData->textWidget, pData->caretChar, pData->cursorIndex, pOutVerts );

	return pOutVerts;
}

static void OnPropertyChanged(struct UIWidget* pThisWidget, struct WidgetPropertyBinding* pBinding)
{

}

static void DoBackspace(struct TextEntryWidgetData* pData, struct UIWidget* pWidget)
{
	if(pData->currentStringLen == 0)
	{
		return;
	}
	// shift along starting with the char at cursorIndex
	for(int i=pData->cursorIndex; i < pData->currentStringLen; i++)
	{
		pData->textWidget.content[i - 1] = pData->textWidget.content[i];
	}
	pData->cursorIndex--;
	pData->textWidget.content[--pData->currentStringLen] = '\0';
	struct WidgetPropertyBinding* pBinding = UI_FindBinding(pWidget, "content");
	if (pBinding)
	{
		char* setterName = UI_MakeBindingSetterFunctionName(pBinding->name);
		struct ScriptCallArgument arg;
		arg.type = SCA_string;
		arg.val.string = pData->textWidget.content;
		Sc_CallFuncInRegTableEntryTable(pWidget->scriptCallbacks.viewmodelTable, setterName, &arg, 1, 0);
		free(setterName);
	}
}

static void DoEnterChar(struct TextEntryWidgetData* pData, char c, struct UIWidget* pWidget)
{
	if(pData->currentStringLen >= pData->maxStringLen)
	{
		return;
	}
	// shift along starting with the char at cursorIndex
	pData->textWidget.content[pData->currentStringLen + 1] = '\0';
	for(int i=pData->currentStringLen - 1; i >= pData->cursorIndex; i--)
	{
		pData->textWidget.content[i + 1] = pData->textWidget.content[i];
	}
	pData->textWidget.content[pData->cursorIndex++] = c;
	pData->currentStringLen++;

	struct WidgetPropertyBinding* pBinding = UI_FindBinding(pWidget, "content");
	if (pBinding)
	{
		char* setterName = UI_MakeBindingSetterFunctionName(pBinding->name);
		struct ScriptCallArgument arg;
		arg.type = SCA_string;
		arg.val.string = pData->textWidget.content;
		Sc_CallFuncInRegTableEntryTable(pWidget->scriptCallbacks.viewmodelTable, setterName, &arg, 1, 0);
		free(setterName);
	}
}

static void RecieveKeystrokeCallback(struct UIWidget* pWidget, int keystroke)
{
	struct TextEntryWidgetData* pData = pWidget->pImplementationData;

	switch(keystroke)
	{
	case KEYSTROKE_LEFT:
		{
			if(pData->cursorIndex != 0)
			{
				pData->cursorIndex--;
			}
		}
		break;
	case KEYSTROKE_RIGHT:
		{
			if(pData->cursorIndex != pData->currentStringLen)
			{
				pData->cursorIndex++;
			}
		}
		break;
	case KEYSTROKE_BACKSPACE:
		{
			if(pData->cursorIndex != 0)
			{
				DoBackspace(pData, pWidget);
			}
		}
		break;
	case KEYSTROKE_ENTER:
		{
			if (pData->bEnterPressCallbackSet)
			{
				Sc_CallFuncInRegTableEntryTable(pData->viewmodelRegIndex, pData->onEnterPressLuaCallbackName, NULL, 0, 0);
			}
		}
		return;
	default:
		DoEnterChar(pData, (char)keystroke, pWidget);
		break;
	}
	SetRootWidgetIsDirty(pData->pLayerData->rootWidget, true);
}

/*
	Initialise the TextEntryWidget and create a bigger allocation than is required it's TextWidget content string, 
	or truncate it to fit withing maxStringLength.
*/
static void AllocateStringContents(int maxStringLength, struct TextEntryWidgetData* pData)
{
	EASSERT(pData->textWidget.content);
	char* newAlloc = malloc(maxStringLength + 1);
	int len = strlen(pData->textWidget.content); 
	if(len >= maxStringLength)
	{
		
		memcpy(newAlloc, pData->textWidget.content, maxStringLength);
		newAlloc[maxStringLength] = '\0';
	}
	else
	{
		strcpy(newAlloc, pData->textWidget.content);
	}
	free(pData->textWidget.content);
	pData->textWidget.content = newAlloc;
	pData->cursorIndex = len;
	pData->currentStringLen = len;

}

static void OnFocus(struct UIWidget* pWidget)
{
	struct TextEntryWidgetData* pData = pWidget->pImplementationData;
	HTimer hCaretTimer = pData->caretTimer;
	pData->pLayerData->timerPool.pPool[hCaretTimer].bActive = true;
	pData->bCaretBlinkState = true;
	SetRootWidgetIsDirty(pData->pLayerData->rootWidget, true);
}

static void OnUnFocus(struct UIWidget* pWidget)
{
	struct TextEntryWidgetData* pData = pWidget->pImplementationData;
	HTimer hCaretTimer = pData->caretTimer;
	pData->pLayerData->timerPool.pPool[hCaretTimer].bActive = false;
	pData->bCaretBlinkState = false;
	SetRootWidgetIsDirty(pData->pLayerData->rootWidget, true);
}

static bool OnCaretBlinkTimerElapsed(struct SDTimer* pTimer)
{
	struct UIWidget* pWidget = UI_GetWidget((HWidget)(u64)pTimer->pUserData);
	struct TextEntryWidgetData* pData = pWidget->pImplementationData;
	pData->bCaretBlinkState = ! pData->bCaretBlinkState;
	SetRootWidgetIsDirty(pData->pLayerData->rootWidget, true);
	static int r = 0;
	return false;
}

static void SetupCaretBlinkTimer(struct TextEntryWidgetData* pData, struct XMLUIData* pUILayerData, HWidget hWidget)
{
	struct SDTimer timer = 
	{
		.bActive = true,
		.bRepeat = true,
		.bAutoReset = true,
		.total = 0.5,

		.fnCallback = &OnCaretBlinkTimerElapsed,
		.pUserData = (void*)(u64)hWidget
	};
	pData->caretTimer = TP_GetTimer(&pUILayerData->timerPool, &timer);
}

static void MakeWidgetIntoTextEntryWidget(HWidget hWidget, struct DataNode* pDataNode, struct XMLUIData* pUILayerData)
{
	struct UIWidget* pWidget = UI_GetWidget(hWidget);
	pWidget->hNext = -1;
	pWidget->hPrev = -1;
	pWidget->hParent = -1;
	pWidget->hFirstChild = -1;
	pWidget->fnGetHeight = &GetHeight;
	pWidget->fnGetWidth = &GetWidth;
	pWidget->fnLayoutChildren = &LayoutChildren;
	pWidget->fnOnDestroy = &OnDestroy;
	pWidget->fnOutputVertices = &OnOutputVerts;
	pWidget->fnOnBoundPropertyChanged = &OnPropertyChanged;
	pWidget->fnRecieveKeystroke = &RecieveKeystrokeCallback;
	pWidget->bAcceptsFocus = true;
	pWidget->pImplementationData = malloc(sizeof(struct TextEntryWidgetData));
	pWidget->cCallbacks.Callbacks[WC_OnGainFocus].callback.focusChangeFn = &OnFocus;
	pWidget->cCallbacks.Callbacks[WC_OnLeaveFocus].callback.focusChangeFn = &OnUnFocus;

	memset(pWidget->pImplementationData, 0, sizeof(struct TextEntryWidgetData));
	struct TextEntryWidgetData* pData = pWidget->pImplementationData;
	
	pData->pLayerData = pUILayerData;
	TextWidget_FromXML(pWidget, &pData->textWidget, pDataNode, pUILayerData);
	pData->canvasWidget.bUseHSlider = false;
	pData->canvasWidget.bUseVSlider = false;
	xmlChar* attribute = NULL;
	
	if(pDataNode->fnGetPropType(pDataNode, "maxStringLength") == DN_Int)
	{
		pData->maxStringLen = pDataNode->fnGetInt(pDataNode, "maxStringLength");
	}
	else
	{
		pData->maxStringLen = 16;
	}

	if (pDataNode->fnGetPropType(pDataNode, "onEnter") == DN_String)
	{
		int nameLen = pDataNode->fnGetStrlen(pDataNode, "onEnter");
		if (nameLen <= TEXT_WIDGET_DATA_LUA_CALLBACK_NAME_BUFFER_SIZE)
		{
			pDataNode->fnGetStrcpy(pDataNode, "onEnter", pData->onEnterPressLuaCallbackName);
			pData->bEnterPressCallbackSet = true;
		}
		else
		{
			char* errorMsgName = malloc(nameLen + 1);
			pDataNode->fnGetStrcpy(pDataNode, "onEnter", errorMsgName);
			Log_Error("TextWidget: onEnter callback name '%s' too long. 31 chars max, name was %i", errorMsgName, nameLen);
			free(errorMsgName);
		}
	}

	AllocateStringContents(pData->maxStringLen, pData);
	pData->caretChar = 'I'; // todo: move to xml
	SetupCaretBlinkTimer(pData, pUILayerData, hWidget);
	HTimer hCaretTimer = pData->caretTimer;
	pData->pLayerData->timerPool.pPool[hCaretTimer].bActive = false;
	pData->bCaretBlinkState = false;
	pData->viewmodelRegIndex = pUILayerData->hViewModel;
}

HWidget TextEntryWidgetNew(HWidget hParent, struct DataNode* pDataNode, struct XMLUIData* pUILayerData)
{
	HWidget hWidget = UI_NewBlankWidget();
	MakeWidgetIntoTextEntryWidget(hWidget, pDataNode, pUILayerData);
	return hWidget;
}