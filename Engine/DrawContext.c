#include "DrawContext.h"
#include <string.h>
DrawContext Dr_InitDrawContext()
{
	DrawContext d;
	memset(&d, 0, sizeof(DrawContext));
	return d;
}

void Dr_OnScreenDimsChange(int newW, int newH)
{
}
