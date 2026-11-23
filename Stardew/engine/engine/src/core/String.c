#include "StardewString.h"

int Str_Tokenize(char* string, char sep)
{
	int outNumTokens = 0;
	if(*string != '\0')
	{
		outNumTokens = 1;
	}
	while(*string != '\0')
	{
		if(*string == sep)
		{
			outNumTokens++;
			*string = '\0';
		}
		string++;
	}
	return outNumTokens;
}

void Str_AdvanceToNextToken(char** pNextToken)
{
	char* pC = *pNextToken;
	while(*pC != '\0') pC++;
	pC++;
	*pNextToken = pC;
}

