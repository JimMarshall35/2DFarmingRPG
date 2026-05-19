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
#include "TimerPool.h"

#define MAX_GAME_LAYER_ASSET_FILE_PATH_LEN 128

typedef struct InputMapping InputMapping;

struct GameFrameworkEventListener;

struct GameLayer2DData;
struct BinarySerializer;
struct Vert2DTexture;
struct LineVertex;
typedef struct LineVertex WorldspaceLineVertex;

typedef void (*PreFirstInitFn)(struct GameLayer2DData* pGameLayerData);
typedef void (*PreLoadLevelFn)(struct GameLayer2DData* pGameLayerData);

/**
	@brief
	Extend the writing of the level data packet that servers send to clients
*/
typedef void (*LevelDataPacketExtenderFn)(struct GameLayer2DData* pGameLayerData, struct BinarySerializer* pBS);

/**
	@brief
	- Extend the handling of a level data request packet that the server reciever
	- the client can send extra data by extending the G2D_Enqueue_RequestLevelData function by calling G2D_Extend_RequestLevelDataMessage and
	registering an extender 
*/
typedef void (*LevelDataRequestHandlerExtenderFn)(struct GameLayer2DData* pGameLayerData, struct BinarySerializer* pBS);

typedef void (*LevelDataHandlerExtenderFn)(struct GameLayer2DData* pGameLayerData, struct BinarySerializer* pBS);


struct GameFrameworkLayer;
struct DrawContext;
typedef struct DrawContext DrawContext;

// the real type of this should be hSprite ie u32 but i want to save memory so u16 it is - that 
// should be enough for anyone - just store the tiles in the first 16 bits worth of indexes


struct Transform2D
{
	vec2 position;
	vec2 scale;
	vec2 rotationPointRelative; /* relative to position */
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
	/** @brief for convenience, a reference back to the layer */
	struct GameFrameworkLayer* pLayer;

	/** @brief The top node of a quad tree that holds references to static sprites for culling */
	HEntity2DQuadtreeNode hEntitiesQuadTree;

	/** @brief handle to atlas containing sprite data */
	hAtlas hAtlas;

	/** @brief tilemap comprised of a list of tilemap layers */
	struct TileMap tilemap;

	/** @brief mostly pointless */
	bool bLoaded;

	/** @brief
		When in this mode the game is paused and you have
		a different set of controls to freely move the camera around,
		in future I might add editing
	*/
	bool bFreeLookMode;

	/** @brief the one and only camera */
	struct Transform2D camera;

	/** @brief
		Which layer, if any, is the camera clamped to.
		If it is not clamped (the default) then this should be -1 
	*/
	int cameraClampedToTilemapLayer;

	/** @brief
		controls for free look mode
	*/
	struct FreeLookCameraModeControls freeLookCtrls;

	/** @brief
		buffers of vertices and indices populated each frame
	*/
	VECTOR(struct Vert2DTexture) pWorldspaceVertices;
	VECTOR(VertIndexT) pWorldspaceIndices;
	H2DWorldspaceVertexBuffer vertexBuffer;

	/** @brief
	 * debug line drawing buffers
	*/
	VECTOR(WorldspaceLineVertex) pWorldspaceLineVertices;
	HWorldspaceLineVertexBuffer lineBuffer;

	/** @brief
		Path of loaded atlas file
	*/
	char atlasFilePath[MAX_GAME_LAYER_ASSET_FILE_PATH_LEN];
	
	/** @brief
		path of loaded level file
	*/
	char tilemapFilePath[MAX_GAME_LAYER_ASSET_FILE_PATH_LEN];

	/** @brief
		flag for whether the debug overlay is pushed on top of the game
	*/
	bool bDebugLayerAttatched;

	/** @brief
		A message to display on the screen when the debug overlay is on top of the game2dlayer
	*/
	char debugMsg[256];

	/** @brief
		Listens for the debug overlay game framework layer being pushed
	*/
	struct GameFrameworkEventListener* pDebugListener;

	/** @brief
		Window width
	*/
	int windowW;

	/** @brief
		Window height
	*/
	int windowH;

	/** @brief
		Physics world handle
	*/
	HPhysicsWorld hPhysicsWorld;

	/** @brief
		Entities collection
	*/
	struct Entity2DCollection entities;

	/** @brief
		Game specific data
	*/
	void* pUserData;

	/** @brief 
		a callback called when the layer is pushed, and the level and all entities are loaded, but have not had their init methods called yet. 
		An opportunity for your game to load sprite handles from the atlas that the entities will use in their init methods.
	*/
	PreFirstInitFn preFirstInitCallback;

	PreLoadLevelFn preLoadLevelFn;

	/** @brief
		HACK :
		todo: sort out the availabilty of these draw and input contexts
		todo: use global getter for draw context
	*/
	DrawContext* pDrawContext;

	/*
		@brief
		don't draw
	*/
	bool bSkipDraw;

	/** @brief
		Current location has changed in such a way that it needs to be saved, when a new area is moved to or you sleep
	*/
	bool bCurrentLocationIsDirty;

	/** @brief
		AS THE SERVER :
		Write extra game specific data to the level data packet the server sends initially to each client
	*/
	LevelDataPacketExtenderFn levelDataPacketExtender;

	/** @brief
		AS THE SERVER :
		Read extra game specific data from the level data request packet the client sends to the server

		- extra data is written by extending the G2D_Enqueue_RequestLevelData function by calling G2D_Extend_RequestLevelDataMessage and
		registering an extender
	*/
	LevelDataRequestHandlerExtenderFn levelDataRequestHandlerExtender;

	/** @brief
		AS THE CLIENT :
		Read extra game specific data the server has written to the initial level data packet
	*/
	LevelDataHandlerExtenderFn levelDataHandlerExtender;

	/// @brief Use this to register timers that will be called after a given time, optionally on repeat
	struct SDTimerPool timerPool; 

	/// @brief handler for timer that calls G2D_Enqueue_Worldstate_Packet
	HTimer hNetworkStateUpdateTimer;
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

/// @brief 
/// @param pData 
/// @param pBS 
void G2D_SaveLevelDataInternal(struct GameLayer2DData* pData, struct BinarySerializer* pBS);


/// @brief Query which dynamic entities are in a given region (their bounding boxes intersect with the region)
/// @param pLayer 
/// @param pCollection 
/// @param regionTL 
/// @param regionBR 
/// @param pOutEntities 
/// @return 
VECTOR(HEntity2D) Et2D_QueryDynEntities(struct GameFrameworkLayer* pLayer, struct Entity2DCollection* pCollection, vec2 regionTL, vec2 regionBR, VECTOR(HEntity2D) pOutEntities);

/// @brief convert a transform2D to a matrix
/// @param pTrans input transform
/// @param out output matrix
void Et2D_Transform2DToMat3(struct Transform2D* pTrans, mat3 out);

#ifdef __cplusplus
}
#endif

#endif // !H_GAME2DLAYER
