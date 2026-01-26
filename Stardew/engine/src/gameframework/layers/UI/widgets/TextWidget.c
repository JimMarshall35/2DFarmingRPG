#include "TextWidget.h"
#include "XMLUIGameLayer.h"
#include "Widget.h"
#include "Atlas.h"
#include "AssertLib.h"
#include <stdio.h>
#include <string.h>
#include "WidgetVertexOutputHelpers.h"
#include "DataNode.h"
#include "Scripting.h"
#include "RootWidget.h"
#include "Log.h"
#include "StardewString.h"

void TextWidget_Destroy(struct TextWidgetData* pData)
{
	free(pData->content);
}


static float GetWidth(struct UIWidget* pWidget, struct UIWidget* pParent)
{
	struct TextWidgetData* pData = pWidget->pImplementationData;
	return Fo_StringWidth(pData->atlas, pData->font, pData->content) + pWidget->padding.paddingLeft + pWidget->padding.paddingRight;
}

static float GetHeight(struct UIWidget* pWidget, struct UIWidget* pParent)
{
	struct TextWidgetData* pData = pWidget->pImplementationData;
	return Fo_StringHeight(pData->atlas, pData->font, pData->content) + pWidget->padding.paddingTop + pWidget->padding.paddingBottom;
}

static void LayoutChildren(struct UIWidget* pWidget, struct UIWidget* pParent)
{

}


static void OnDestroy(struct UIWidget* pWidget)
{
	struct TextWidgetData* pData = pWidget->pImplementationData;
	TextWidget_Destroy(pData);
	free(pData);
}



void* TextWidget_OutputVerts(float left, float top, const struct WidgetPadding* padding, struct TextWidgetData* pData, VECTOR(WidgetVertex) pOutVerts)
{
	DECLARE_STATIC_STRING_COPY(sCopyBuffer, pData->content)
	int numLines = Str_Tokenize(sCopyBuffer, '\n');
	char* currentLine = sCopyBuffer;

	float maxYBearing = Fo_GetMaxYBearing(pData->atlas, pData->font, pData->content);
	vec2 pen = { left + padding->paddingLeft, top + maxYBearing + padding->paddingTop };
	vec2 start = { pen[0], pen[1] };
	for (int i = 0; i < len; i++)
	{
		char c = pData->content[i];
		if(c == '\n')
		{
			float height = Fo_StringHeightSingleLine(pData->atlas, pData->font, currentLine);
			Str_AdvanceToNextToken(&currentLine);
			pen[0] = start[0];
			pen[1] += height;
			//continue;
		}
		AtlasSprite* pAtlasSprite = Fo_GetCharSprite(pData->atlas, pData->font, c);
		vec2 bearing = { 0,0 };
		vec2 advance = { 0.0, 0.0 };
		vec2 output = { 0,0 };
		vec2 bearingApplied = { 0,0 };

		if (!pAtlasSprite)
		{
			Log_Warning("can't get atlas sprite, file TextWidget.c");
			EASSERT(false);
			return pOutVerts;
		}

		EVERIFY(Fo_TryGetCharBearing(pData->atlas, pData->font, c, bearing));
		bearing[1] *= -1.0f; // FT coordinate system for bearing has increasing Y as up, in our game coordinate system that is decreasing y 
		EVERIFY(Fo_TryGetCharAdvance(pData->atlas, pData->font, c, advance));
		WidgetQuad quad;
		PopulateWidgetQuadWholeSprite(&quad, pAtlasSprite);
		SetWidgetQuadColour(&quad, pData->r, pData->g, pData->b, pData->a);

		// topleft
		glm_vec2_add(bearing, pen, output);
		TranslateWidgetQuad(output, &quad);

		pOutVerts = OutputWidgetQuad(pOutVerts, &quad);
		glm_vec2_add(advance, pen, pen);
	}

	return pOutVerts;
}

void* TextWidget_OutputAtLetter(float left, float top, const struct WidgetPadding* padding, struct TextWidgetData* pData, char charOverlay, int letterOverlay, VECTOR(WidgetVertex) pOutVerts)
{
	//struct TextWidgetData* pData = pThisWidget->pImplementationData;
	DECLARE_STATIC_STRING_COPY(sCopyBuffer, pData->content)
	int numLines = Str_Tokenize(sCopyBuffer, '\n');
	char* currentLine = sCopyBuffer;

	float maxYBearing = Fo_GetMaxYBearing(pData->atlas, pData->font, pData->content);
	vec2 pen = { left + padding->paddingLeft, top + maxYBearing + padding->paddingTop };
	vec2 start = { pen[0], pen[1] };
	for (int i = 0; i < len; i++)
	{
		char c = pData->content[i];
		if(c == '\n')
		{
			float height = Fo_StringHeightSingleLine(pData->atlas, pData->font, currentLine);
			Str_AdvanceToNextToken(&currentLine);
			pen[0] = start[0];
			pen[1] += height;
			continue;
		}
		AtlasSprite* pAtlasSprite = Fo_GetCharSprite(pData->atlas, pData->font, c);
		vec2 bearing = { 0,0 };
		vec2 advance = { 0.0, 0.0 };
		vec2 output = { 0,0 };
		vec2 bearingApplied = { 0,0 };

		if (!pAtlasSprite)
		{
			Log_Warning("can't get atlas sprite, file TextWidget.c");
			EASSERT(false);
			return pOutVerts;
		}

		EVERIFY(Fo_TryGetCharBearing(pData->atlas, pData->font, c, bearing));
		bearing[1] *= -1.0f; // FT coordinate system for bearing has increasing Y as up, in our game coordinate system that is decreasing y 
		EVERIFY(Fo_TryGetCharAdvance(pData->atlas, pData->font, c, advance));
		WidgetQuad quad;
		PopulateWidgetQuadWholeSprite(&quad, pAtlasSprite);
		SetWidgetQuadColour(&quad, pData->r, pData->g, pData->b, pData->a);

		// topleft
		glm_vec2_add(bearing, pen, output);
		TranslateWidgetQuad(output, &quad);
		output[1] = top;
		if(i == letterOverlay)
		{
			WidgetQuad quad;
			AtlasSprite* pAtlasSpriteOverlay = Fo_GetCharSprite(pData->atlas, pData->font, charOverlay);
			PopulateWidgetQuadWholeSprite(&quad, pAtlasSpriteOverlay);
			SetWidgetQuadColour(&quad, pData->r, pData->g, pData->b, pData->a);
			TranslateWidgetQuad(output, &quad);
			pOutVerts = OutputWidgetQuad(pOutVerts, &quad);
		}
		glm_vec2_add(advance, pen, pen);
	}
	
	return pOutVerts;
}


static void* OnOutputVerts(struct UIWidget* pThisWidget, VECTOR(WidgetVertex) pOutVerts)
{
	pOutVerts = TextWidget_OutputVerts(pThisWidget->left, pThisWidget->top, &pThisWidget->padding, pThisWidget->pImplementationData, pOutVerts);
	pOutVerts = UI_Helper_OnOutputVerts(pThisWidget, pOutVerts);
	return pOutVerts;
}

static void ParseColourAttribute(char* inText, struct TextWidgetData* pOutWidgetData)
{
	char* tok = strtok(inText, ",");
	int onToken = 0;
	while (tok)
	{
		int i = atoi(tok);
		EASSERT(i < 256);
		switch (onToken++)
		{
		case 0:
			pOutWidgetData->r = (float)i / 255.0f;
			break;
		case 1:
			pOutWidgetData->g = (float)i / 255.0f;
			break;
		case 2:
			pOutWidgetData->b = (float)i / 255.0f;
			break;
		case 3:
			pOutWidgetData->a = (float)i / 255.0f;
			break;
		default:
			Log_Error("ParseColourAttribute: invalid number of tokens: %i", onToken);
		}
		tok = strtok(NULL, ",");
	}
}

static void ParseSizeAttribute(char* inText, struct TextWidgetData* pOutWidgetData)
{
	char* endPtr;
	float val = strtof(inText, &endPtr);
	if (strcmp(endPtr, "pxls") == 0)
	{
		val = At_PixelsToPts(val);
	}
	pOutWidgetData->fSizePts = val;
}

static bool IsWhitespaceChar(char character)
{
	return 
		character == ' '  ||
		character == '\n' ||
		character == '\t' ||
		character == '\r';
}

/*
	params:
		inString - input string to strip
		outStrippedStart - output ptr to starting character
	returns:
		length after stripping start and end whitespace
*/
static int GetWhitespaceStrippedLengthAndStart(char* inString, char** outStrippedStart)
{
	char* strippedStart = inString;
	char* strippedEnd = NULL;
	
	// find start ptr
	while(IsWhitespaceChar(*strippedStart)) 
	{
		strippedStart++;
	}
	*outStrippedStart = strippedStart;

	// find end
	int len = strlen(inString);
	strippedEnd = inString + (len - 1);
	while(IsWhitespaceChar(*strippedEnd))
	{
		strippedEnd--;
	}
	return (strippedEnd - strippedStart) + 1;
}

/*
	returns a string that is the input string with trailing and leading whitespace stripped.
	Callers responsibility to free with free()
*/
static char* GetWhitespaceStrippedString(char* inString)
{
	char* pStart = NULL;
	int strippedLen = GetWhitespaceStrippedLengthAndStart(inString, &pStart);
	char* outStr = malloc(strippedLen + 1);
	memcpy(outStr, pStart, strippedLen);
	outStr[strippedLen] = '\0';
	return outStr;
}

void TextWidget_FromXML(struct UIWidget* pWidget, struct TextWidgetData* pData, struct DataNode* pXMLNode, struct XMLUIData* pUILayerData)
{
	pData->atlas = pUILayerData->atlas;
	char* fontName = NULL;
	char* str = NULL;
	size_t len = pXMLNode->fnGetContentStrlen(pXMLNode);
	if(len)
	{
		str = malloc(len + 1);
		pXMLNode->fnGetContentStrcpy(pXMLNode, str);
	}
	else
	{
		str = malloc(1);
		str[0] = '\0';
	}
	if(pData->content)
	{
		free(pData->content);
		pData->content = NULL;
	}
	pData->content = GetWhitespaceStrippedString(str);
	if(UI_IsAttributeStringABindingExpression(pData->content))
	{
		char* p = NULL;
		UI_AddStringPropertyBinding(
			pWidget,
			"content",
			pData->content,
			&p,
			pUILayerData->hViewModel);
		free(pData->content);
		pData->content = p;
	}

	free(str);



	bool bFontSet = false;
	bool bFontSizeSet = false;
	if(pXMLNode->fnGetPropType(pXMLNode, "font") == DN_String)
	{
		size_t fontNameLen = pXMLNode->fnGetStrlen(pXMLNode, "font");
		fontName = malloc(fontNameLen + 1);
		pXMLNode->fnGetStrcpy(pXMLNode, "font", fontName);
		bFontSet = true;
	}
	if(pXMLNode->fnGetPropType(pXMLNode, "colour") == DN_String)
	{
		size_t colourLen = pXMLNode->fnGetStrlen(pXMLNode, "colour");
		char* colourStr = malloc(colourLen + 1);
		pXMLNode->fnGetStrcpy(pXMLNode, "colour", colourStr);
		ParseColourAttribute(colourStr, pData);
		free(colourStr);
	}
	if(pXMLNode->fnGetPropType(pXMLNode, "fontSize") == DN_String)
	{
		size_t sizeLen = pXMLNode->fnGetStrlen(pXMLNode, "fontSize");
		char* sizeStr = malloc(sizeLen + 1);
		pXMLNode->fnGetStrcpy(pXMLNode, "fontSize", sizeStr);
		ParseSizeAttribute(sizeStr, pData);
		bFontSizeSet = true;
		free(sizeStr);
	}
	
	if (bFontSet && bFontSizeSet)
	{
		HFont font = Fo_FindFont(pUILayerData->atlas, fontName, pData->fSizePts);
		if (font == NULL_HANDLE)
		{
			Log_Error("TextWidget_FromXML: can't find font %s size %f pts", fontName, pData->fSizePts);
			EASSERT(false);
		}
		pData->font = font;
	}
	if(fontName)
	{
		free(fontName);
	}
}

static void OnPropertyChanged(struct UIWidget* pThisWidget, struct WidgetPropertyBinding* pBinding)
{
	struct TextWidgetData* pData = pThisWidget->pImplementationData;
	if (strcmp(pBinding->boundPropertyName, "content") == 0)
	{
		char* fnName = UI_MakeBindingGetterFunctionName(pBinding->name);
		Sc_CallFuncInRegTableEntryTable(pThisWidget->scriptCallbacks.viewmodelTable, fnName, NULL, 0, 1);
		free(fnName);
		if (pData->content)
		{
			free(pData->content);
		}
		pData->content = malloc(Sc_StackTopStringLen() + 1);
		Sc_StackTopStrCopy(pData->content);
		Sc_ResetStack();
		SetRootWidgetIsDirty(pData->rootWidget, true);
	}
}


static void MakeWidgetIntoTextWidget(HWidget hWidget, struct DataNode* pDataNode, struct XMLUIData* pUILayerData)
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
	pWidget->pImplementationData = malloc(sizeof(struct TextWidgetData));

	memset(pWidget->pImplementationData, 0, sizeof(struct TextWidgetData));

	struct TextWidgetData* pData = pWidget->pImplementationData;
	pData->rootWidget = pUILayerData->rootWidget;
	TextWidget_FromXML(pWidget, pData, pDataNode, pUILayerData);

}

HWidget TextWidgetNew(HWidget hParent, struct DataNode* pDataNode, struct XMLUIData* pUILayerData)
{
	HWidget hWidget = UI_NewBlankWidget();
	MakeWidgetIntoTextWidget(hWidget, pDataNode, pUILayerData);
	return hWidget;
}
