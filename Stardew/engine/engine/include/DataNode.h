/**
    @file DataNode.h
    @brief
    Provides an abstraction layer between widget constructors and the data they need
    which can come from either an xml node or from the lua table on the top of the 
    lua stack (global for now) 
*/
#ifndef DATANODE_H
#define DATANODE_H

#include <stdbool.h>
#include <stdlib.h>

typedef struct _xmlNode xmlNode;
struct DataNode;

enum DNPropValType
{
    DN_Int,
    DN_Float,
    DN_Bool,
    DN_String,
    DN_UNKNOWN,
    DN_PROP_NOT_FOUND,
};


typedef enum DNPropValType (*GetDNPropTypeFn)(struct DataNode* pNode, const char* propName);
typedef float (*GetDNFloatFn)(struct DataNode* pNode, const char* propName);
typedef int (*GetDNIntFn)(struct DataNode* pNode, const char* propName);
typedef bool (*GetDNBoolFn)(struct DataNode* pNode, const char* propName);
typedef size_t (*GetDNStrlenFn)(struct DataNode* pNode, const char* propName);
typedef void (*GetDNStrcpyFn)(struct DataNode* pNode, const char* propName, char* dest);

/** @brief
    returns true if the property named propName on pNode is a string and matches cmpTo
*/
typedef bool (*GetDNStrCmpFn)(struct DataNode* pNode, const char* propName, const char* cmpTo);


/** @brief
    For the XML node implementation, these functions operate on the content of the node itself.
    For the lua one they operatoe on a property called "content"
*/
typedef size_t (*GetDNStrlenContentFn)(struct DataNode* pNode);
typedef void (*GetDNStrcpyContentFn)(struct DataNode* pNode, char* dest);
typedef bool (*GetDNStrCmpContentFn)(struct DataNode* pNode, const char* cmpTo);

void DN_InitForXMLNode(struct DataNode* pOutNode, xmlNode* pXMLNode);
void DN_InitForLuaTableOnTopOfStack(struct DataNode* pOutNode);

struct DataNode
{
    GetDNPropTypeFn fnGetPropType;
    GetDNFloatFn fnGetFloat;
    GetDNIntFn fnGetInt;
    GetDNBoolFn fnGetBool;
    GetDNStrlenFn fnGetStrlen;
    GetDNStrcpyFn fnGetStrcpy;
    GetDNStrCmpFn fnStrCmp;
    GetDNStrlenContentFn fnGetContentStrlen;
    GetDNStrcpyContentFn fnGetContentStrcpy;
    GetDNStrCmpContentFn fnContentStrCmp;

    /// @brief xmlNode* or NULL for lua table on top of stack
    void* pData; 
};

#endif