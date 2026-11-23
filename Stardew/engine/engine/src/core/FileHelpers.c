#include "FileHelpers.h"
#include <stdio.h>
#include <stdlib.h>
#include "AssertLib.h"

char* LoadFile(const char* path, int* outSize)
{
	FILE* fp = fopen(path, "r");
	if (!fp) return NULL;
	fseek(fp, 0L, SEEK_END);
	*outSize = ftell(fp);
	int sz = *outSize;
	fseek(fp, 0L, SEEK_SET);
	void* pOut = malloc(sz);
	EASSERT(pOut);
	size_t r = fread(pOut, 1, sz, fp);
	EASSERT(r > 0);
	fclose(fp);
	return pOut;
}
