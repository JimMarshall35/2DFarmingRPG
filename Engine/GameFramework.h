#ifndef GAMEFRAMEWORK_H
#define GAMEFRAMEWORK_H
#include "InputContext.h"
#include "DrawContext.h"

typedef void (*UpdateFn)(float deltaT);
typedef void (*DrawFn)(DrawContext* context);
typedef void (*InputFn)(InputContext* context);
typedef void (*OnPushFn)(void);
typedef void (*OnPopFn)(void);

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

struct GameFrameworkLayer
{
	UpdateFn update;
	DrawFn draw;
	InputFn input;
	OnPushFn onPush;
	OnPopFn onPop;
	unsigned int flags;
};

void InitGameFramework();
void DestroyGameFramework();

void PushGameFrameworkLayer(const struct GameFrameworkLayer* layer);
void PopGameFrameworkLayer();

void UpdateGameFramework(float deltaT);
void InputGameFramework(InputContext* context);
void DrawGameFramework(DrawContext* context);

#endif
