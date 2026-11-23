#include "GameFramework.h"
#include "InputContext.h"
#include "DynArray.h"
#include "DrawContext.h"
#include "AssertLib.h"
#include "GameFrameworkEvent.h"
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "Profile.h"

struct LayerChange
{
	bool bIsPush;
	struct GameFrameworkLayer layer;
};

static VECTOR(struct GameFrameworkLayer) gLayerStack = NULL;

static VECTOR(struct LayerChange) gLayerChangeQueue = NULL;
static bool gChangeLastFrame = false;
static int gLayerPopCount = 0;

static int gInputItrStart = 0;
static int gUpdateItrStart = 0;
static int gDrawItrStart = 0;


void GF_InitGameFramework()
{
	ZONE(ctx)
	gLayerStack = NEW_VECTOR(struct GameFrameworkLayer);
	gLayerStack = VectorResize(gLayerStack, 32);
	gLayerStack = VectorClear(gLayerStack);
	gLayerChangeQueue = NEW_VECTOR(struct LayerChange);
	Ev_Init();
	ZONE_END(ctx)
}

void GF_DestroyGameFramework()
{
	ZONE(ctx)
	DestoryVector(gLayerStack);
	ZONE_END(ctx)
}

void GF_PushGameFrameworkLayer(const struct GameFrameworkLayer* layer)
{
	ZONE(ctx)
	struct LayerChange lc;
	lc.bIsPush = true;
	memcpy(&lc.layer, layer, sizeof(struct GameFrameworkLayer));
	gLayerChangeQueue = VectorPush(gLayerChangeQueue, &lc);
	ZONE_END(ctx)
}

static int FindNewItrStart(GameFrameworkLayerFlags type)
{
	ZONE(ctx)
	int rval = 0;
	for (int i = VectorSize(gLayerStack) - 1; i <= 0; i--)
	{
		rval = i;
		if (gLayerStack[rval].flags & type)
		{
			break;
		}
	}
	ZONE_END(ctx)
	return rval;
}
 
void GF_PopGameFrameworkLayer()
{
	ZONE(ctx)
	struct LayerChange lc;
	lc.bIsPush = false;
	memset(&lc.layer, 0, sizeof(struct GameFrameworkLayer));
	gLayerChangeQueue = VectorPush(gLayerChangeQueue, &lc);
	ZONE_END(ctx)
}

void GF_EndFrame(DrawContext* drawContext, InputContext* inputContext)
{
	ZONE(ctx)
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
				struct GameFrameworkLayer* pTop = VectorTop(gLayerStack);
				pTop->onPush(pTop, drawContext, inputContext);
			}
			gChangeLastFrame = true;
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
				pLayer->onPop(pLayer, drawContext, inputContext);
			}

			VectorPop(gLayerStack);
			gChangeLastFrame = true;
		}
		//VectorPop(gLayerChangeQueue);
	}
	gLayerChangeQueue = VectorClear(gLayerChangeQueue);
	ZONE_END(ctx)
}

void GF_UpdateGameFramework(float deltaT)
{
	ZONE(ctx)
	for (int i = gUpdateItrStart; i < VectorSize(gLayerStack); i++)
	{
		if(gLayerStack[i].flags & EnableUpdateFn)
		{
			gLayerStack[i].update(&gLayerStack[i], deltaT);
		}
	}
	ZONE_END(ctx)
}

void GF_InputGameFramework(InputContext* context)
{
	ZONE(ctx)
	for (int i = gInputItrStart; i < VectorSize(gLayerStack); i++)
	{
		if(gLayerStack[i].flags & EnableInputFn)
		{
			gLayerStack[i].input(&gLayerStack[i], context);
		}
	}
	ZONE_END(ctx)
}

void GF_DrawGameFramework(DrawContext* context)
{
	// if(gChangeLastFrame)
	// {
	// 	gChangeLastFrame = false;
	// 	return;
	// }
	ZONE(ctx)
	int c = 0;
	for (int i = gDrawItrStart; i < VectorSize(gLayerStack); i++)
	{
		if(gLayerStack[i].flags & EnableDrawFn)
		{
			gLayerStack[i].draw(&gLayerStack[i], context);
		}
	}
	ZONE_END(ctx)
}

void GF_OnWindowDimsChanged(int newW, int newH)
{
	ZONE(ctx)
	for (int i = 0; i < VectorSize(gLayerStack); i++)
	{
		if (gLayerStack[i].onWindowDimsChanged)
		{
			gLayerStack[i].onWindowDimsChanged(&gLayerStack[i], newW, newH);
		}
	}
	ZONE_END(ctx)
}

struct GameFrameworkLayer* GF_GetLayerBelow(struct GameFrameworkLayer* pLayer)
{
	ZONE(ctx)
	EASSERT(VectorSize(gLayerStack) > 0);
	EASSERT(pLayer != NULL);

	struct GameFrameworkLayer* pLast = NULL;
	for (int i = VectorSize(gLayerStack) -1; i >= 0; i--)
	{
		if (pLast == pLayer)
		{
			ZONE_END(ctx)
			return &gLayerStack[i];
		}
		pLast = &gLayerStack[i];
	}
	ZONE_END(ctx)
	return NULL;
}