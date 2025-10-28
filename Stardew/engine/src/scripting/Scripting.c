#include "Scripting.h"
#include "lua.h"
#include <lualib.h>
#include <lauxlib.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "AssertLib.h"
#include "XMLUIGameLayer.h"
#include "RootWidget.h"
#include "AssertLib.h"
#include "GameFrameworkEvent.h"
#include "DataNode.h"

#define GAME_LUA_MINOR_VERSION 1

static lua_State* gL = NULL;

static void OnPropertyChangedInternal(XMLUIData* pUIData, HWidget hWidget, const char* pChangedPropName)
{
	//printf("OnPropertyChangedInternal. pChangedPropName: %s\n", pChangedPropName);

	while (hWidget != NULL_HWIDGET)
	{
		struct UIWidget* pWidget = UI_GetWidget(hWidget);
		for (int i = 0; i < pWidget->numBindings; i++)
		{
			struct WidgetPropertyBinding* pBinding = &pWidget->bindings[i];
			if (strcmp(pBinding->name, pChangedPropName) == 0)
			{
				if(strcmp(pBinding->boundPropertyName, "childrenBinding") == 0)
				{
					printf("childrenBinding changed. pChangedPropName: %s\n", pChangedPropName);
					char* pStr = malloc(strlen(pChangedPropName) + 1);
					printf("malloc\n");

					strcpy(pStr, pChangedPropName);
					printf("pStr: %s\n", pStr);

					struct WidgetChildrenChangeRequest r = {
						pUIData->hViewModel,
						pStr,
						hWidget
					};
					printf("pUIData->pChildrenChangeRequests: %p\n", pUIData->pChildrenChangeRequests);
					volatile VectorData* pDbg = VectorData_DEBUG(pUIData->pChildrenChangeRequests);
					pUIData->pChildrenChangeRequests = VectorPush(pUIData->pChildrenChangeRequests, &r);
					pDbg = VectorData_DEBUG(pUIData->pChildrenChangeRequests);
					printf("pushed request\n");

				}
				else if (pWidget->fnOnBoundPropertyChanged)
				{
					pWidget->fnOnBoundPropertyChanged(pWidget, pBinding);
				}
			}
		}
		OnPropertyChangedInternal(pUIData, pWidget->hFirstChild, pChangedPropName);

		hWidget = pWidget->hNext;
	}
}

struct LuaListenerUserData
{
	int regIndexFn;
	int regIndexVmTable;
	int numArgs;
};

static void LuaListenerFn(void* pUserData, void* pEventData)
{
	struct LuaListenerUserData* ud = pUserData;
	struct LuaListenedEventArgs* pArgs = pEventData;
	Sc_CallFuncInRegTableEntry(ud->regIndexFn, pArgs->args, pArgs->numArgs, 0, ud->regIndexVmTable);
}

static int L_FireGameFrameworkEvent(lua_State* L)
{
	const char* eventName = NULL;
	if(lua_isstring(L, -1))
	{
		eventName = lua_tostring(L, -1);
	}
	else
	{
		printf("FireGameFrameworkEvent. First argument wrong type should be string\n");
	}
	if(lua_istable(L,-2))
	{
		lua_pop(gL, 1);
		struct DataNode node;
		DN_InitForLuaTableOnTopOfStack(&node);
		// will this work? I don't know. Is L the same as gL? I hope so
		Ev_FireEvent((char*)eventName, &node);
		Sc_ResetStack();
	}
	else
	{
		printf("FireGameFrameworkEvent. 2nd argument wrong type should be table\n");
	}
	return 0;
}

static int L_UnSubscribeToGameFrameworkEvent(lua_State* L)
{
	if(lua_islightuserdata(L, -1))
	{
		struct GameFrameworkEventListener* pListener = lua_touserdata(L, -1);

		struct LuaListenerUserData* ud = Ev_GetUserData(pListener);
		luaL_unref(gL, LUA_REGISTRYINDEX, ud->regIndexFn);
		luaL_unref(gL, LUA_REGISTRYINDEX, ud->regIndexVmTable);
		free(ud);
		EVERIFY(Ev_UnsubscribeEvent(pListener));
		lua_settop(L, 0);
		lua_pushboolean(L, true);
		return 1;
	}
	else
	{
		printf("UnsubscribeFromGameFrameworkEvent - argument wrong type\n");
		lua_settop(L, 0);
		lua_pushboolean(L, false);
		return 1;
	}
}

static int L_SubscribeToGameFrameworkEvent(lua_State* L)
{
	// lua args:
	// 	event name, self, listenerLuaFunction
	int regIndexFn = 0;
	int regIndexVm = 0;
	struct LuaListenerUserData* ud = malloc(sizeof(struct LuaListenerUserData));
	memset(ud, 0, sizeof(struct LuaListenerUserData));
	
	if (lua_isfunction(L, -1))
	{
		ud->regIndexFn = luaL_ref(L, LUA_REGISTRYINDEX);
	}
	else
	{
		printf("SubscribeGameFrameworkEvent arg 3 wrong type. Is type. Needs to be string\n");
		lua_settop(L, 0);
		lua_pushnil(L);
		free(ud);
		return 1;
	}

	if(lua_istable(L,-1))
	{
		ud->regIndexVmTable = luaL_ref(L, LUA_REGISTRYINDEX);
	}
	else
	{
		printf("SubscribeGameFrameworkEvent arg 3 wrong type. Is type. Needs to be string\n");
		lua_settop(L, 0);
		lua_pushnil(L);
		free(ud);
		return 1;
	}

	if (lua_isstring(L, -1))
	{
		struct GameFrameworkEventListener* pListener = Ev_SubscribeEvent((char*)lua_tostring(L, -1), &LuaListenerFn, ud);
		lua_settop(L, 0);
		lua_pushlightuserdata(L, pListener);
		return 1;
	}
	else
	{
		printf("SubscribeGameFrameworkEvent arg 3 wrong type. Is type. Needs to be string\n");
		lua_settop(L, 0);
		lua_pushnil(L);
		free(ud);
		return 1;
	}
}

static int L_OnPropertyChanged(lua_State* L)
{
	// args: (table) viewmodel, (string) propertyName
	int top = lua_gettop(L);
	EASSERT(top == 2);
	bool bIsTable = lua_istable(L, 1);
	EASSERT(bIsTable);
	const char* pStr = luaL_checkstring(L, 2);
	char* pNameCpy = malloc(strlen(pStr) + 1);
	strcpy(pNameCpy, pStr);
	lua_pop(L, 1);
	lua_getfield(L, -1, "XMLUIDataPtr");
	XMLUIData* pUIData = lua_touserdata(L, -1);
	HWidget hWidget = pUIData->rootWidget;
	OnPropertyChangedInternal(pUIData, hWidget, pNameCpy);
	free(pNameCpy);
	SetRootWidgetIsDirty(pUIData->rootWidget, true);
	return 0;
}

void Sc_RegisterCFunction(const char* name, int(*fn)(lua_State*))
{
	lua_pushcfunction(gL, fn);
	lua_setglobal(gL, name);
}

void Sc_InitScripting()
{
	gL = luaL_newstate();
	luaL_openlibs(gL); /* Load Lua libraries */
	//lua_pushcfunction(gL, &L_OnPropertyChanged);
	//lua_setglobal(gL, "OnPropertyChanged");
	Sc_RegisterCFunction("OnPropertyChanged", &L_OnPropertyChanged);
	Sc_RegisterCFunction("SubscribeGameFrameworkEvent", &L_SubscribeToGameFrameworkEvent);
	Sc_RegisterCFunction("UnsubscribeGameFrameworkEvent", &L_UnSubscribeToGameFrameworkEvent);
	Sc_RegisterCFunction("FireGameFrameworkEvent", &L_FireGameFrameworkEvent);
}

void Sc_DeInitScripting()
{
	lua_close(gL);
}

bool Sc_OpenFile(const char* path)
{
	int status = luaL_loadfile(gL, path);
	if (status)
	{
		/* If something went wrong, error message is at the top of */
		/* the stack */
		fprintf(stderr, "Couldn't load file: %s\n", lua_tostring(gL, -1));
		return false;
	}
	lua_pcall(gL, 0, LUA_MULTRET, 0);
	return status == 0;
#if GAME_LUA_VERSION > 1
	return status == LUA_OK;
#endif
}

static const char* GetTypeOnTopOfStack()
{
	if (lua_isnil(gL, -1))
	{
		return "nil";
	}
	else if (lua_isnumber(gL, -1))
	{
		return "number";
	}
	else if (lua_isstring(gL, -1))
	{
		return "string";
	}
	else if (lua_isboolean(gL, -1))
	{
		return "boolean";
	}
	else if (lua_istable(gL, -1))
	{
		return "table";
	}
	else if (lua_isuserdata(gL, -1))
	{
		return "userdata";
	}
	return "unknown";
}

static void PushFunctionCallArgsOntoStack(struct ScriptCallArgument* pArgs, int numArgs)
{
	for (int i = 0; i < numArgs; i++)
	{
		struct ScriptCallArgument* pArg = &pArgs[i];
		switch (pArg->type)
		{
		case SCA_nil:
			lua_pushnil(gL);
			break;
		case SCA_boolean:
			lua_pushboolean(gL, pArg->val.boolean);
			break;
		case SCA_number:
			lua_pushnumber(gL, pArg->val.number);
			break;
		case SCA_string:
			lua_pushstring(gL, pArg->val.string);
			break;
		case SCA_table:
			lua_rawgeti(gL, LUA_REGISTRYINDEX, pArg->val.table);
			EASSERT(lua_istable(gL, -1));
			break;
		case SCA_userdata:
			lua_pushlightuserdata(gL, pArg->val.userData);
			break;
		case SCA_int:
			lua_pushinteger(gL, pArg->val.i);
			break;
		}
	}
}

int Sc_CallGlobalFuncReturningTableAndStoreResultInReg(const char* funcName, struct ScriptCallArgument* pArgs, int numArgs)
{
	int rVal = 0;
	lua_getglobal(gL, funcName);
	if (lua_isfunction(gL, -1))
	{
		PushFunctionCallArgsOntoStack(pArgs, numArgs);
		const int returnvalues_count = 1; // function returns 0 values

		lua_pcall(gL, numArgs, returnvalues_count, 0); // now call the function
		EASSERT(lua_istable(gL, -1));
		rVal = luaL_ref(gL, LUA_REGISTRYINDEX);
	}
	else
	{
		printf("Sc_CallGlobalFuncReturningTableAndStoreResultInReg funcName '%s' was not a function, it was type '%s'\n", funcName, GetTypeOnTopOfStack());
	}
	lua_settop(gL, 0);
	return rVal;
}

void Sc_CallFuncInRegTableEntryTable(int regIndex, const char* funcName, struct ScriptCallArgument* pArgs, int numArgs, int numReturnVals)
{
	lua_rawgeti(gL, LUA_REGISTRYINDEX, regIndex);
	bool bIstable = lua_istable(gL, -1);
	if (!bIstable)
	{
		printf("Sc_CallFuncInRegTableEntryTable. Reg table entry %i is not a table, but %s\n", regIndex, GetTypeOnTopOfStack());
		lua_settop(gL, 0);
		return;
	}
	lua_pushstring(gL, funcName);
	lua_gettable(gL, -2);
	if (lua_isfunction(gL, -1))
	{
		lua_rawgeti(gL, LUA_REGISTRYINDEX, regIndex);
		PushFunctionCallArgsOntoStack(pArgs, numArgs);
		lua_pcall(gL, numArgs + 1, numReturnVals, 0);
	}
	else
	{
		printf("object at key '%s' not a function but type %s \n", funcName, GetTypeOnTopOfStack());
	}
	//lua_settop(gL, 0);
}

void Sc_CallFuncInRegTableEntry(int regIndex, struct ScriptCallArgument* pArgs, int numArgs, int numReturnVals, int selfRegIndex)
{
	lua_rawgeti(gL, LUA_REGISTRYINDEX, regIndex);
	bool bIsFunc = lua_isfunction(gL, -1);
	if (!bIsFunc)
	{
		printf("Sc_CallFuncInRegTableEntry. Reg table entry %i is not a function, but %s\n", regIndex, GetTypeOnTopOfStack());
		lua_settop(gL, 0);
		return ;
	}
	lua_rawgeti(gL, LUA_REGISTRYINDEX, selfRegIndex);

	PushFunctionCallArgsOntoStack(pArgs, numArgs);
	lua_pcall(gL, numArgs + 1, numReturnVals, 0);
	lua_settop(gL, 0);
	return ;
}

void Sc_AddLightUserDataValueToTable(int regIndex, const char* userDataKey, void* userDataValue)
{
	lua_rawgeti(gL, LUA_REGISTRYINDEX, regIndex);
	bool bIstable = lua_istable(gL, -1);
	if (!bIstable)
	{
		printf("Sc_CallFuncInRegTableEntryTable. Reg table entry %i is not a table, but %s\n", regIndex, GetTypeOnTopOfStack());
		lua_settop(gL, 0);
		return;
	}
	lua_pushlightuserdata(gL, userDataValue);
	lua_setfield(gL, -2, userDataKey);
	lua_settop(gL, 0);
}

bool Sc_FunctionPresentInTable(int regIndex, const char* funcName)
{
	lua_rawgeti(gL, LUA_REGISTRYINDEX, regIndex);
	bool bIstable = lua_istable(gL, -1);
	if (!bIstable)
	{
		printf("Sc_FunctionPresentInTable. Reg table entry %i is not a table, but %s\n", regIndex, GetTypeOnTopOfStack());
		lua_settop(gL, 0);
		return;
	}
	Sc_TableGet(funcName);
	bool bPresent = !Sc_IsNil();
	lua_settop(gL, 0);
	return bPresent;
}

void Sc_DumpStack()
{
	int top = lua_gettop(gL);
	for (int i = 1; i <= top; i++) 
	{
		printf("%d\t%s\t", i, luaL_typename(gL, i));
		switch (lua_type(gL, i)) 
		{
		case LUA_TNUMBER:
			printf("%g\n", lua_tonumber(gL, i));
			break;
		case LUA_TSTRING:
			printf("%s\n", lua_tostring(gL, i));
			break;
		case LUA_TBOOLEAN:
			printf("%s\n", (lua_toboolean(gL, i) ? "true" : "false"));
			break;
		case LUA_TNIL:
			printf("%s\n", "nil");
			break;
		default:
			printf("%p\n", lua_topointer(gL, i));
			break;
		}
	}
}

int Sc_Int()
{
#if GAME_LUA_MINOR_VERSION >= 3
	EASSERT(lua_isinteger(gL, -1));
#endif
	return lua_tointeger(gL, -1);
}

float Sc_Float()
{
	EASSERT(lua_isnumber(gL, -1));
	return lua_tonumber(gL, -1);
}

size_t Sc_StackTopStringLen()
{
	EASSERT(lua_isstring(gL, -1));
	const char* str = lua_tostring(gL, -1);
	return strlen(str);
}

void Sc_StackTopStrCopy(char* pOutString)
{
	EASSERT(lua_isstring(gL, -1));
	const char* str = lua_tostring(gL, -1);
	strcpy(pOutString, str);
}

void Sc_ResetStack()
{
	lua_settop(gL, 0);
}

void Sc_DeleteTableInReg(int index)
{
	luaL_unref(gL, LUA_REGISTRYINDEX, index);
}

bool Sc_IsTable()
{
	return lua_istable(gL, -1);
}

int Sc_Type()
{
	return lua_type(gL, -1);
}

void Sc_Pop()
{
	lua_pop(gL, 1);
}

void Sc_TableGet(const char* key)
{
	EASSERT(Sc_IsTable());
	lua_getfield(gL, -1, key);
}

void Sc_TableGetIndex(int index)
{
	EASSERT(Sc_IsTable());
	lua_geti(gL, -1, index);
}

int Sc_TableLen()
{
	EASSERT(Sc_IsTable());
	lua_len(gL, -1);
	int i = Sc_Int();
	Sc_Pop();
	return i;
}

bool Sc_IsNil()
{
	return lua_isnil(gL, -1);	
}

bool Sc_IsString()
{
	return lua_isstring(gL, -1);
}

bool Sc_IsInteger()
{
	return lua_isinteger(gL, -1);
}

bool Sc_IsBool()
{
	return lua_isboolean(gL, -1);
}

bool Sc_IsNumber()
{
	return lua_isnumber(gL, -1);
}

bool Sc_Bool()
{
	return lua_toboolean(gL, -1) != 0;
}

bool Sc_IsFunction()
{
	return lua_isfunction(gL, -1) != 0;
}

bool Sc_StringCmp(const char* cmpTo)
{
	EASSERT(Sc_IsString());
	const char* str = lua_tostring(gL, -1);
	return strcmp(str, cmpTo) == 0;
}

void Sc_NewTableOnStack(int arrayElementHint, int nonArrayElementHint)
{
	lua_createtable(gL, arrayElementHint, nonArrayElementHint);
}

void Sc_SetIntAtTableIndex(int index, int value)
{
	EASSERT(Sc_IsTable());
	lua_pushinteger(gL, index);
	lua_pushinteger(gL, value);
	lua_settable(gL, -3);
}

void Sc_SetTable()
{
	lua_settable(gL, -3);
}

void Sc_PushInt(int i)
{
	lua_pushinteger(gL, i);
}


void Sc_SetIntAtTableKey(const char* key, int val)
{
	EASSERT(Sc_IsTable());
	lua_pushinteger(gL, val);
	lua_setfield(gL, -2, key);
}

int Sc_RefTable()
{
	return luaL_ref(gL, LUA_REGISTRYINDEX);
}

void Sc_UnRefTable(int ref)
{
	luaL_unref(gL, LUA_REGISTRYINDEX, ref);
}