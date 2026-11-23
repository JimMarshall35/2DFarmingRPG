#ifndef GAMEFRAMEWORK_H
#define GAMEFRAMEWORK_H

#ifdef __cplusplus
extern "C"{
#endif 

#include "InputContext.h"
#include "DrawContext.h"

struct GameFrameworkLayer;

typedef void (*UpdateFn)(struct GameFrameworkLayer* pLayer, float deltaT);
typedef void (*DrawFn)(struct GameFrameworkLayer* pLayer, DrawContext* context);
typedef void (*InputFn)(struct GameFrameworkLayer* pLayer, InputContext* context);
typedef void (*OnPushFn)(struct GameFrameworkLayer* pLayer, DrawContext* drawContext, InputContext* inputContext);
typedef void (*OnPopFn)(struct GameFrameworkLayer* pLayer, DrawContext* drawContext, InputContext* inputContext);
typedef void (*OnWindowDimsChangedFn)(struct GameFrameworkLayer* pLayer, int newW, int newH);

typedef enum
{
	EnableUpdateFn = 1,
	EnableDrawFn = 2,
	EnableInputFn = 8,
	EnableOnPush = 16,
	EnableOnPop = 32,
	MasksDraw = 64,
	MasksUpdate = 128,
	MasksInput = 256
}GameFrameworkLayerFlags;

#define GF_ANYMASKMASK (MasksInput | MasksUpdate | MasksDraw)

enum GameFrameworkLayerType
{
	GFT_Unknown,
	GFT_UI,
	GFT_Game2D,
};

struct GameFrameworkLayer
{
	UpdateFn update;
	DrawFn draw;
	InputFn input;
	OnPushFn onPush;
	OnPopFn onPop;
	OnWindowDimsChangedFn onWindowDimsChanged;
	unsigned int flags;
	/// @brief it is the game framework users responsiblity to alloc and free
	void* userData; 
	enum GameFrameworkLayerType type;
};


void GF_InitGameFramework();
void GF_DestroyGameFramework();

void GF_PushGameFrameworkLayer(const struct GameFrameworkLayer* layer);
void GF_PopGameFrameworkLayer();

void GF_EndFrame(DrawContext* drawContext, InputContext* inputContext);

void GF_UpdateGameFramework(float deltaT);
void GF_InputGameFramework(InputContext* context);
void GF_DrawGameFramework(DrawContext* context);
void GF_OnWindowDimsChanged(int newW, int newH);

/**
	@brief
	Returns NULL if no layer below
*/
struct GameFrameworkLayer* GF_GetLayerBelow(struct GameFrameworkLayer* pLayer);

#ifdef __cplusplus
}
#endif

#endif
