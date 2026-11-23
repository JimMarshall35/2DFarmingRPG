#include "DataNode.h"
#include <libxml/tree.h>
#include "Scripting.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "AssertLib.h"
#include "Log.h"

static bool ContainsChar(const char* str, char c)
{
    while (*str)
    {
        if (*str == c)
        {
            return true;
        }
        str++;
    }
    return false;
}

static enum DNPropValType GetPropType_XMLNode(struct DataNode* pNode, const char* propName)
{
    xmlNode* pXMLNode = (xmlNode*)pNode->pData;
    xmlChar* prop = xmlGetProp(pXMLNode, (const xmlChar*)propName);
    if (prop == NULL)
    {
        return DN_PROP_NOT_FOUND;
    }
    // rudimentary type inference
    // if it looks like an int, its an int
    // if it looks like a float, its a float
    // if it looks like true/false its a bool
    // otherwise its a string
    char* endPtr = NULL;

    endPtr = NULL;
    float floatVal = strtof((const char*)prop, &endPtr);
    if (endPtr != (const char*)prop && *endPtr == '\0')
    {
        /* its a float if it has a . or an e in it, OK?! OK then... */
        if(ContainsChar((const char*)prop, '.') || ContainsChar((const char*)prop, 'e'))
        {
            xmlFree(prop);
            return DN_Float;
        }
    }

    endPtr = NULL;
    long intVal = strtol((const char*)prop, &endPtr, 10);
    if (endPtr != (const char*)prop && *endPtr == '\0')
    {
        xmlFree(prop);
        return DN_Int;
    }
    
    if (strcmp((const char*)prop, "true") == 0 || strcmp((const char*)prop, "false") == 0)
    {
        xmlFree(prop);
        return DN_Bool;
    }
    xmlFree(prop);
    return DN_String;
}

static float GetFloat_XML(struct DataNode* pNode, const char* propName)
{
    xmlNode* pXMLNode = (xmlNode*)pNode->pData;
    xmlChar* prop = xmlGetProp(pXMLNode, (const xmlChar*)propName);
    if (prop == NULL)
    {
        return -999999999.0;//0.0f / 0.0f; // nan
    }
    char* endPtr = NULL;
    float floatVal = strtof((const char*)prop, &endPtr);
    return floatVal;
}

static int GetInt_XML(struct DataNode* pNode, const char* propName)
{
    xmlNode* pXMLNode = (xmlNode*)pNode->pData;
    xmlChar* prop = xmlGetProp(pXMLNode, (const xmlChar*)propName);
    if (prop == NULL)
    {
        return 0;
    }
    char* endPtr = NULL;
    long intVal = strtol((const char*)prop, &endPtr, 10);
    return (int)intVal;
}

static bool GetBool_XML(struct DataNode* pNode, const char* propName)
{
    xmlNode* pXMLNode = (xmlNode*)pNode->pData;
    xmlChar* prop = xmlGetProp(pXMLNode, (const xmlChar*)propName);
    if (prop == NULL)
    {
        return false;
    }
    bool val = false;
    if (strcmp((const char*)prop, "true") == 0)
    {
        val = true;
    }
    else if(strcmp((const char*)prop, "false") == 0)
    {
        val = false;
    }
    else
    {
        Log_Error("GetBool_XML: property %s value %s is not true or false", propName, (const char*)prop);
    }
    xmlFree(prop);
    return false;
}

static size_t GetStrlen_XML(struct DataNode* pNode, const char* propName)
{
    xmlNode* pXMLNode = (xmlNode*)pNode->pData;
    xmlChar* prop = xmlGetProp(pXMLNode, (const xmlChar*)propName);
    if (prop == NULL)
    {
        return 0;
    }
    size_t len = strlen((const char*)prop);
    xmlFree(prop);
    return len;
}

static void GetStrcpy_XML(struct DataNode* pNode, const char* propName, char* dest)
{
    xmlNode* pXMLNode = (xmlNode*)pNode->pData;
    xmlChar* prop = xmlGetProp(pXMLNode, (const xmlChar*)propName);
    if (prop == NULL)
    {
        dest[0] = '\0';
        return;
    }
    strcpy(dest, (const char*)prop);
    xmlFree(prop);
}

bool StrCmp_XML(struct DataNode* pNode, const char* propName, const char* cmpTo)
{
    xmlNode* pXMLNode = (xmlNode*)pNode->pData;
    xmlChar* prop = xmlGetProp(pXMLNode, (const xmlChar*)propName);
    if (prop == NULL)
    {
        return false;
    }
    bool r = strcmp(prop, cmpTo) == 0;
    xmlFree(prop);
    return r;
}

/*
typedef size_t (*GetDNStrlenContentFn)(struct DataNode* pNode);
typedef void (*GetDNStrcpyContentFn)(struct DataNode* pNode, char* dest);
typedef bool (*GetDNStrCmpContentFn)(struct DataNode* pNode, const char* cmpTo);
*/

size_t StrLenContent_XML(struct DataNode* pNode)
{
    xmlNode* pXMLNode = (xmlNode*)pNode->pData;
    xmlChar* content = xmlNodeGetContent(pXMLNode);
    if (content == NULL)
    {
        return 0;
    }
    size_t len = strlen((const char*)content);
    xmlFree(content);
    return len;
}

void StrCpyContent_XML(struct DataNode* pNode, char* dest)
{
    xmlNode* pXMLNode = (xmlNode*)pNode->pData;
    xmlChar* content = xmlNodeGetContent(pXMLNode);
    if (content == NULL)
    {
        return;
    }
    strcpy(dest, (const char*)content);
    xmlFree(content);
}

bool StrCmpContent_XML(struct DataNode* pNode, const char* cmpTo)
{
    xmlNode* pXMLNode = (xmlNode*)pNode->pData;
    xmlChar* content = xmlNodeGetContent(pXMLNode);
    if (content == NULL)
    {
        return false;
    }
    bool r = strcmp((const char*)content, cmpTo) == 0;
    xmlFree(content);
    return r;
}

void DN_InitForXMLNode(struct DataNode* pOutNode, xmlNode* pXMLNode)
{
    pOutNode->fnGetPropType = &GetPropType_XMLNode;
    pOutNode->fnGetFloat = &GetFloat_XML;
    pOutNode->fnGetInt = &GetInt_XML;
    pOutNode->fnGetBool = &GetBool_XML;
    pOutNode->fnGetStrlen = &GetStrlen_XML;
    pOutNode->fnGetStrcpy = &GetStrcpy_XML;
    pOutNode->fnStrCmp = &StrCmp_XML;
    pOutNode->fnGetContentStrlen = &StrLenContent_XML;
    pOutNode->fnGetContentStrcpy = &StrCpyContent_XML;
    pOutNode->fnContentStrCmp = &StrCmpContent_XML;
    pOutNode->pData = pXMLNode;
}

static enum DNPropValType GetPropType_Lua(struct DataNode* pNode, const char* propName)
{
    Sc_TableGet(propName);
    if (Sc_IsNil())
    {
        Sc_Pop();
        return DN_PROP_NOT_FOUND;
    }
    else if (Sc_IsInteger())
    {
        Sc_Pop();
        return DN_Int;
    }
    else if (Sc_IsBool())
    {
        Sc_Pop();
        return DN_Bool;
    }
    else if (Sc_IsNumber())
    {
        Sc_Pop();
        return DN_Float;
    }
    else if (Sc_IsString())
    {
        Sc_Pop();
        return DN_String;
    }
    
    Sc_Pop();
    return DN_UNKNOWN;
}

static float GetFloat_Lua(struct DataNode* pNode, const char* propName)
{
    Sc_TableGet(propName);
    EASSERT(Sc_IsNumber());
    float val = (float)Sc_Float();
    Sc_Pop();
    return val;
}

static int GetInt_Lua(struct DataNode* pNode, const char* propName)
{
    Sc_TableGet(propName);
    EASSERT(Sc_IsInteger());
    int val = Sc_Int();
    Sc_Pop();
    return val;
}

static bool GetBool_Lua(struct DataNode* pNode, const char* propName)
{
    Sc_TableGet(propName);
    EASSERT(Sc_IsBool());
    bool val = Sc_Bool();
    Sc_Pop();
    return val;
}

static size_t GetStrlen_Lua(struct DataNode* pNode, const char* propName)
{
    Sc_TableGet(propName);
    EASSERT(Sc_IsString());
    size_t len = Sc_StackTopStringLen();
    Sc_Pop();
    return len;
}

static void GetStrcpy_Lua(struct DataNode* pNode, const char* propName, char* dest)
{
    Sc_TableGet(propName);
    EASSERT(Sc_IsString());
    Sc_StackTopStrCopy(dest);
    Sc_Pop();
}

static bool StrCmp_Lua(struct DataNode* pNode, const char* propName, const char* cmpTo)
{
    Sc_TableGet(propName);
    if (!Sc_IsString())
    {
        Log_Error("StrCmp_Lua: property %s is not a string", propName);
        Sc_Pop();
        return false;
    }
    bool r = Sc_StringCmp(cmpTo);
    Sc_Pop();
    return r;
}

size_t StrLenContent_Lua(struct DataNode* pNode)
{
    Sc_TableGet("content");
    EASSERT(Sc_IsString());
    size_t len = Sc_StackTopStringLen();
    Sc_Pop();
    return len;
}

void StrCpyContent_Lua(struct DataNode* pNode, char* dest)
{
    Sc_TableGet("content");
    EASSERT(Sc_IsString());
    Sc_StackTopStrCopy(dest);
    Sc_Pop();
}
bool StrCmpContent_Lua(struct DataNode* pNode, const char* cmpTo)
{
    Sc_TableGet("content");
    EASSERT(Sc_IsString());
    bool r = Sc_StringCmp(cmpTo);
    Sc_Pop();
    return r;
}

void DN_InitForLuaTableOnTopOfStack(struct DataNode* pOutNode)
{
    pOutNode->fnGetPropType = &GetPropType_Lua;
    pOutNode->fnGetFloat = &GetFloat_Lua;
    pOutNode->fnGetInt = &GetInt_Lua;
    pOutNode->fnGetBool = &GetBool_Lua;
    pOutNode->fnGetStrlen = &GetStrlen_Lua;
    pOutNode->fnGetStrcpy = &GetStrcpy_Lua;
    pOutNode->fnStrCmp = &StrCmp_Lua;
    pOutNode->fnGetContentStrlen = &StrLenContent_Lua;
    pOutNode->fnGetContentStrcpy = &StrCpyContent_Lua;
    pOutNode->fnContentStrCmp = &StrCmpContent_Lua;
    pOutNode->pData = NULL;
}

