#ifndef DRAWCONTEXT_H
#define DRAWCONTEXT_H
#ifdef __cplusplus
extern "C" {
#endif

#include "Atlas.h"
#include "HandleDefs.h"

struct TileMap;

struct Vert2DTexture
{
	float x, y;
	float u, v;
};

struct Vert2DTextureQuad
{
	struct Vert2DTexture v[4];
};

typedef struct Vert2DTexture Worldspace2DVert;
typedef struct Vert2DTextureQuad Worldspace2DQuad;

struct Vert2DColourTexture
{
	float x, y;
	float u, v;
	float r, g, b, a;
};

struct Vert2DColourTextureQuad
{
	struct Vert2DColourTexture v[4];
};

typedef struct Vert2DColourTexture WidgetVertex;
typedef struct Vert2DColourTextureQuad WidgetQuad;

typedef void(*SetCurrentAtlasFn)(hTexture atlas);
typedef HUIVertexBuffer(*NewUIVertexBufferFn)(int size);
typedef void(*UIVertexBufferDataFn)(HUIVertexBuffer hBuf, WidgetVertex* src, size_t size);
typedef void(*DrawUIVertexBufferFn)(HUIVertexBuffer hBuf, size_t vertexCount);
typedef void(*DestroyUIVertexBufferFn)(HUIVertexBuffer hBuf);
typedef hTexture(*UploadTextureFn)(void* src, int channels, int pxWidth, int pxHeight);
typedef void(*DestroyTextureFn)(hTexture tex);


typedef H2DWorldspaceVertexBuffer(*NewWorldspaceVertBufferFn)(int size);

typedef u32 VertIndexT;
typedef void(*WorldspaceVertexBufferDataFn)(H2DWorldspaceVertexBuffer hBuf, Worldspace2DVert* src, size_t size, VertIndexT* indices, u32 numIndices);
typedef void(*DrawWorldspaceVertexBufferFn)(H2DWorldspaceVertexBuffer hBuf, size_t vertexCount, mat4 view);
typedef void(*DestroyWorldspaceVertexBufferFn)(H2DWorldspaceVertexBuffer hBuf);

typedef void(*ClearScreenFn)();


typedef struct DrawContext
{
	int screenWidth;
	int screenHeight;
	NewUIVertexBufferFn NewUIVertexBuffer;
	UIVertexBufferDataFn UIVertexBufferData;
	DrawUIVertexBufferFn DrawUIVertexBuffer;
	DestroyUIVertexBufferFn DestroyVertexBuffer;
	SetCurrentAtlasFn SetCurrentAtlas;
	UploadTextureFn UploadTexture;
	DestroyTextureFn DestroyTexture;

	NewWorldspaceVertBufferFn NewWorldspaceVertBuffer;
	WorldspaceVertexBufferDataFn WorldspaceVertexBufferData;
	DrawWorldspaceVertexBufferFn DrawWorldspaceVertexBuffer;
	DestroyWorldspaceVertexBufferFn DestroyWorldspaceVertexBuffer;
	ClearScreenFn ClearScreen;
}DrawContext;

DrawContext Dr_InitDrawContext();
void Dr_OnScreenDimsChange(DrawContext* pCtx, int newW, int newH);

#ifdef __cplusplus
}
#endif


#endif