#ifndef SCRIPTING_H
#define SCRIPTING_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdbool.h>
#include "IntTypes.h"

#define MAX_SCRIPT_FUNCTION_NAME_SIZE 32

struct ScriptCallArgument;

struct LuaListenedEventArgs
{
	struct ScriptCallArgument* args;
	int numArgs;
};

enum ScriptCallArgumentDataType
{
	SCA_nil, SCA_boolean, SCA_number, SCA_string, SCA_userdata, SCA_table, SCA_int

};

struct ScriptCallArgument
{
	enum ScriptCallArgumentDataType type;
	union
	{
		bool boolean;
		double number;
		char* string;
		void* userData;
		int table; // in our case it will be a table stored int he regsitry with this value as a key
		int i;
	}val;
};

void Sc_InitScripting();
void Sc_DeInitScripting();
bool Sc_OpenFile(const char* path);

/// <summary>
/// 
/// </summary>
/// <param name="funcName"></param>
/// <param name="pArgs"></param>
/// <param name="numArgs"></param>
/// <param name="pAllocationFor">
/// 
/// What C memory allocation is the tables lifetime tied to. this value will be used for the reg table key.
/// This is global and shared among other lua libraries, to get a unique key to store things in we use the pointer for a C memory 
/// allocation. When the memory is deallocated the table should be deleted by calling Sc_DeleteTableInReg
/// 
/// </param>
/// <returns> key to the table returned, stored in the registry table. 0 if failed </returns>
int Sc_CallGlobalFuncReturningTableAndStoreResultInReg(const char* funcName, struct ScriptCallArgument* pArgs, int numArgs);

void Sc_CallFuncInRegTableEntryTable(int regIndex, const char* funcName, struct ScriptCallArgument* pArgs, int numArgs, int numReturnVals);

void Sc_CallFuncInRegTableEntry(int regIndex, struct ScriptCallArgument* pArgs, int numArgs, int numReturnVals, int selfRegIndex);

void Sc_AddLightUserDataValueToTable(int regIndex, const char* userDataKey, void* userDataValue);

bool Sc_FunctionPresentInTable(int regIndex, const char* funcName);

size_t Sc_StackTopStringLen();
void Sc_StackTopStrCopy(char* pOutString);
void Sc_ResetStack();

/// <summary>
/// 
/// </summary>
/// <param name="index"> delete a table stored in the lua registry </param>
void Sc_DeleteTableInReg(int index);

int Sc_Int();
float Sc_Float();
bool Sc_Bool();

bool Sc_IsTable();
bool Sc_IsNil();
bool Sc_IsString();
bool Sc_IsInteger();
bool Sc_IsBool();
bool Sc_IsNumber();
int Sc_Type();


void Sc_Pop();
void Sc_TableGet(const char* key);
void Sc_TableGetIndex(int index);
int Sc_TableLen();

bool Sc_StringCmp(const char* cmpTo);

void Sc_DumpStack();

/*
	I only intend this to be used for unit testing, and in any games linking to the engine
*/
typedef struct lua_State lua_State;
void Sc_RegisterCFunction(const char* name, int(*fn)(lua_State*));

/* will add these as and when needed for the game */
void Sc_NewTableOnStack(int arrayElementHint, int nonArrayElementHint);
void Sc_SetIntAtTableIndex(int index, int value);
void Sc_SetIntAtTableKey(const char* key, int val);
int Sc_RefTable();
void Sc_UnRefTable(int ref);

/* 
	(stack on table)
	push index onto stack
	push something to set at the index
	call this to set the value at the index
*/
void Sc_SetTable();

void Sc_PushInt(int i);

#ifdef __cplusplus
}
#endif

#endif
