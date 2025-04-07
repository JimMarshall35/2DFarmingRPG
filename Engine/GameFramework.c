#include "GameFramework.h"
#include "InputContext.h"
#include "DynArray.h"
#include "DrawContext.h"
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

struct LayerChange
{
	bool bIsPush;
	struct GameFrameworkLayer layer;
};

static VECTOR(struct GameFrameworkLayer) gLayerStack = NULL;

static VECTOR(struct LayerChange) gLayerChangeQueue = NULL;
static int gLayerPopCount = 0;

static int gInputItrStart = 0;
static int gUpdateItrStart = 0;
static int gDrawItrStart = 0;


void GF_InitGameFramework()
{
	gLayerStack = NEW_VECTOR(struct GameFrameworkLayer);
	gLayerChangeQueue = NEW_VECTOR(struct LayerChange);

}

void GF_DestroyGameFramework()
{
	DestoryVector(gLayerStack);
}

void GF_PushGameFrameworkLayer(const struct GameFrameworkLayer* layer)
{
	/*gLayerStack = VectorPush(gLayerStack, layer);
	if (layer->flags & MasksInput)
	{
		gInputItrStart = VectorSize(gLayerStack) - 1;
	}
	if (layer->flags & MasksUpdate)
	{
		gUpdateItrStart = VectorSize(gLayerStack) - 1;
	}
	if (layer->flags & MasksDraw)
	{
		gDrawItrStart = VectorSize(gLayerStack) - 1;
	}
	if (layer->flags & EnableOnPush)
	{
		layer->onPush();
	}*/
	struct LayerChange lc;
	lc.bIsPush = true;
	memcpy(&lc.layer, layer, sizeof(struct GameFrameworkLayer));
	gLayerChangeQueue = VectorPush(gLayerChangeQueue, &lc);
}

static int FindNewItrStart(GameFrameworkLayerFlags type)
{
	int rval = 0;
	for (int i = VectorSize(gLayerStack) - 1; i <= 0; i--)
	{
		rval = i;
		if (gLayerStack[rval].flags & type)
		{
			break;
		}
	}
	return rval;
}
 
void GF_PopGameFrameworkLayer()
{
	/*struct GameFrameworkLayer* pLayer = VectorTop(gLayerStack);
	if (pLayer->flags & MasksDraw)
	{
		gDrawItrStart = FindNewItrStart(MasksDraw);
	}
	if (pLayer->flags & MasksInput)
	{
		gInputItrStart = FindNewItrStart(MasksInput);
	}
	if (pLayer->flags & MasksUpdate)
	{
		gUpdateItrStart = FindNewItrStart(MasksUpdate);
	}
	if (pLayer->flags & EnableOnPop)
	{
		pLayer->onPop();
	}

	VectorPop(gLayerStack);*/
	struct LayerChange lc;
	lc.bIsPush = false;
	memset(&lc.layer, 0, sizeof(struct GameFrameworkLayer));
	gLayerChangeQueue = VectorPush(gLayerChangeQueue, &lc);

}

void GF_EndFrame()
{
	for (int i = 0; i < VectorSize(gLayerChangeQueue); i++)
	{
		if (gLayerChangeQueue[i].bIsPush)
		{
			gLayerStack = VectorPush(gLayerStack, &gLayerChangeQueue[i].layer);
			if (gLayerChangeQueue[i].layer.flags & MasksInput)
			{
				gInputItrStart = VectorSize(gLayerStack) - 1;
			}
			if (gLayerChangeQueue[i].layer.flags & MasksUpdate)
			{
				gUpdateItrStart = VectorSize(gLayerStack) - 1;
			}
			if (gLayerChangeQueue[i].layer.flags & MasksDraw)
			{
				gDrawItrStart = VectorSize(gLayerStack) - 1;
			}
			if (gLayerChangeQueue[i].layer.flags & EnableOnPush)
			{
				gLayerChangeQueue[i].layer.onPush();
			}
		}
		else
		{
			struct GameFrameworkLayer* pLayer = VectorTop(gLayerStack);
			if (pLayer->flags & MasksDraw)
			{
				gDrawItrStart = FindNewItrStart(MasksDraw);
			}
			if (pLayer->flags & MasksInput)
			{
				gInputItrStart = FindNewItrStart(MasksInput);
			}
			if (pLayer->flags & MasksUpdate)
			{
				gUpdateItrStart = FindNewItrStart(MasksUpdate);
			}
			if (pLayer->flags & EnableOnPop)
			{
				pLayer->onPop();
			}

			VectorPop(gLayerStack);
		}
		gLayerChangeQueue = VectorPop(gLayerChangeQueue);
	}
}

void GF_UpdateGameFramework(float deltaT)
{
	for (int i = gUpdateItrStart; i < VectorSize(gLayerStack); i++)
	{
		gLayerStack[i].update(deltaT);
	}
}

void GF_InputGameFramework(InputContext* context)
{
	for (int i = gInputItrStart; i < VectorSize(gLayerStack); i++)
	{
		gLayerStack[i].input(context);
	}
}

void GF_DrawGameFramework(DrawContext* context)
{
	for (int i = gDrawItrStart; i < VectorSize(gLayerStack); i++)
	{
		gLayerStack[i].draw(context);
	}
}
