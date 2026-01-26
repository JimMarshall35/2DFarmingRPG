#ifndef STARDEWSTRING_H
#define STARDEWSTRING_H

#include "DynArray.h"

/// @brief replaces occurences of the char "sep" in the string with null characters and returns the resulting number of strings.
/// @param string 
/// @param sep separator
/// @return number of tokens
int Str_Tokenize(char* string, char sep);

/// @brief advance a char pointer to the null terminator and then past its
/// @param pNextToken 
void Str_AdvanceToNextToken(char** pNextToken);

/// @brief Declares and lazily intializes a buffer and copies src string into it, call at start of function passing in an arg
/// so that you can tokenize the string with minimal dynamic memory allocation. uses a vector, but doesn't set the vectors size property, just uses it as a string buffer
/// @param copyName name of string copy variable 
/// @param copySrc source string
#define DECLARE_STATIC_STRING_COPY(copyName, copySrc) \
    static VECTOR(char) copyName = NULL; \
	if(!copyName) \
	{ \
		copyName = NEW_VECTOR(char); \
		copyName = VectorResize(copyName, 32000); \
	} \
	size_t len = strlen(copySrc); \
	while(len > VectorCapacity(copyName)) \
	{ \
		copyName = VectorResize(copyName, VectorCapacity(copyName) * 2); \
	} \
	strcpy(copyName, copySrc); 

#endif