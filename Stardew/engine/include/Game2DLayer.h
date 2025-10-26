#ifndef H_GAME2DLAYER
#define H_GAME2DLAYER

#ifdef __cplusplus
extern "C" {
#endif

#include "IntTypes.h"
#include "DynArray.h"
#include "HandleDefs.h"
#include <cglm/cglm.h>
#include "InputContext.h"
#include "FreeLookCameraMode.h"
#include "Entity2DCollection.h"

#define MAX_GAME_LAYER_ASSET_FILE_PATH_LEN 128

typedef struct InputMapping InputMapping;

struct GameFrameworkEventListener;

struct GameLayer2DData;

typedef void (*PreFirstInitFn)(struct GameLayer2DData* pGameLayerData);

struct GameFrameworkLayer;
struct DrawContext;
typedef struct DrawContext DrawContext;

// the real type of this should be hSprite ie u32 but i want to save memory so u16 it is - that 
// should be enough for anyone - just store the tiles in the first 16 bits worth of indexes


struct Transform2D
{
	vec2 position;
	vec2 scale;
	float rotation;
	bool bMirrored;
};

struct TilemapRenderData;
struct TilemapLayerRenderData;

enum ObjectLayer2DDrawOrder
{
	DrawOrder_TopDown,
	DrawOrder_Index	
};

struct TileMapLayer
{
	struct TilemapLayerRenderData* pRenderData;
	struct Transform2D transform;
	int tileWidthPx;
	int tileHeightPx;
	int widthTiles;
	int heightTiles;
	bool bIsObjectLayer;
	enum ObjectLayer2DDrawOrder drawOrder;
	TileIndex* Tiles;
	u32 type;
};

struct TileMap
{
	VECTOR(struct TileMapLayer) layers;
	char* dataFilePath;
	struct TilemapRenderData* pRenderData;
};

struct GameLayer2DData
{
	/* for convenience, a reference back to the layer */
	struct GameFrameworkLayer* pLayer;

	/* The top node of a quad tree that holds references to static sprites for culling */
	HEntity2DQuadtreeNode hEntitiesQuadTree;

	/* handle to atlas containing sprite data */
	hAtlas hAtlas;

	/* tilemap comprised of a list of tilemap layers */
	struct TileMap tilemap;

	/* mostly pointless */
	bool bLoaded;

	/*
		When in this mode the game is paused and you have
		a different set of controls to freely move the camera around,
		in future I might add editing
	*/
	bool bFreeLookMode;

	/* the one and only camera */
	struct Transform2D camera;

	/*
		Which layer, if any, is the camera clamped to.
		If it is not clamped (the default) then this should be -1 
	*/
	int cameraClampedToTilemapLayer;

	/*
		controls for free look mode
	*/
	struct FreeLookCameraModeControls freeLookCtrls;

	/*
		buffers of vertices and indices populated each frame
	*/
	VECTOR(struct Worldspace2DVert) pWorldspaceVertices;
	VECTOR(VertIndexT) pWorldspaceIndices;
	H2DWorldspaceVertexBuffer vertexBuffer;

	/*
		Path of loaded atlas file
	*/
	char atlasFilePath[MAX_GAME_LAYER_ASSET_FILE_PATH_LEN];
	
	/*
		path of loaded level file
	*/
	char tilemapFilePath[MAX_GAME_LAYER_ASSET_FILE_PATH_LEN];

	/*
		flag for whether the debug overlay is pushed on top of the game
	*/
	bool bDebugLayerAttatched;

	/*
		A message to display on the screen when the debug overlay is on top of the game2dlayer
	*/
	char debugMsg[256];

	/*
		Listens for the debug overlay game framework layer being pushed
	*/
	struct GameFrameworkEventListener* pDebugListener;

	/*
		Window width
	*/
	int windowW;

	/*
		Window height
	*/
	int windowH;

	/*
		Physics world handle
	*/
	HPhysicsWorld hPhysicsWorld;

	/*
		Entities collection
	*/
	struct Entity2DCollection entities;

	/*
		Game specifi data
	*/
	void* pUserData;

	/* 
		a callback called when the layer is pushed, and the level and all entities are loaded, but have not had their init methods called yet. 
		An opportunity for your game to load sprite handles from the atlas that the entities will use in their init methods.
	*/
	PreFirstInitFn preFirstInitCallback;

	/*
		HACK:
		todo sort out the availabilty of these draw and input contexts
	*/
	DrawContext* pDrawContext;
};

struct Game2DLayerOptions
{

	const char* atlasFilePath;
	
	const char* levelFilePath;
	
};

void TilemapLayer_GetTLBR(vec2 tl, vec2 br, struct TileMapLayer* pTMLayer);

void Game2DLayer_Get(struct GameFrameworkLayer* pLayer, struct Game2DLayerOptions* pOptions, DrawContext* pDC);

void Game2DLayer_SaveLevelFile(struct GameLayer2DData* pData, const char* outputFilePath);

void GameLayer2D_OnPush(struct GameFrameworkLayer* pLayer, DrawContext* drawContext, InputContext* inputContext);

void Game2DLayer_OnPop(struct GameFrameworkLayer* pLayer, DrawContext* drawContext, InputContext* inputContext);

#ifdef __cplusplus
}
#endif

#endif // !H_GAME2DLAYER
