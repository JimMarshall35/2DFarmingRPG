#include "Game2DLayer.h"
#include "GameFramework.h"
#include "Atlas.h"
#include <stdlib.h>
#include <string.h>
#include "BinarySerializer.h"
#include "DrawContext.h"
#include "AssertLib.h"
#include "DynArray.h"
#include <cglm/cglm.h>
#include "GameFrameworkEvent.h"
#include "Scripting.h"
#include "XMLUIGameLayer.h"
#include "FreeLookCameraMode.h"
#include "EntityQuadTree.h"
#include "FloatingPointLib.h"
#include "Camera2D.h"
#include "Network.h"
#include "Log.h"
#include "Game2DLayerNetwork.h"

int gTilesRendered = 0;

static void LoadTilesUncompressedV1(struct TileMapLayer* pLayer, struct BinarySerializer* pBS)
{
	int allocSize = pLayer->heightTiles * pLayer->widthTiles * sizeof(TileIndex);
	pLayer->Tiles = malloc(allocSize);
	BS_BytesRead(pBS, allocSize, (char*)pLayer->Tiles);
}

void TilemapLayer_GetTLBR(vec2 tl, vec2 br, struct TileMapLayer* pTMLayer)
{

}

static void LoadTilesRLEV1(struct TileMapLayer* pTileMap, struct BinarySerializer* pBS)
{
	EASSERT(false);
}

static void LoadLevelDataV1(struct TileMap* pTileMap, struct BinarySerializer* pBS, struct GameLayer2DData* pData)
{
	float tlx, tly, brx, bry;
	BS_DeSerializeFloat(&tlx, pBS);
	BS_DeSerializeFloat(&tly, pBS);
	BS_DeSerializeFloat(&brx, pBS);
	BS_DeSerializeFloat(&bry, pBS);
	struct Entity2DQuadTreeInitArgs initArgs = {
		.x = tlx, .y = tly,
		.w = brx - tlx,
		.h = bry - tly
	};
	pData->hEntitiesQuadTree = GetEntity2DQuadTree(&initArgs);

	u32 numLayers = 0;
	u32 objectLayer = 0;
	BS_DeSerializeU32(&numLayers, pBS);
	Log_Verbose("Num layers %i", numLayers);
	pTileMap->layers = VectorResize(pTileMap->layers, numLayers);
	for (int i = 0; i < numLayers; i++)
	{
		struct TileMapLayer layer;
		u32 type = 0;
		BS_DeSerializeU32(&type, pBS);
		layer.type = type;
		switch(type)
		{
		case 1: // tile layer
			Log_Verbose("Deserializing tile layer...");
			u32 width, height, x, y, compression, tw, th;
			BS_DeSerializeU32(&width, pBS);
			BS_DeSerializeU32(&height, pBS);
			BS_DeSerializeU32(&x, pBS);
			BS_DeSerializeU32(&y, pBS);
			BS_DeSerializeU32(&tw, pBS);
			BS_DeSerializeU32(&th, pBS);
			BS_DeSerializeU32(&compression, pBS);
			layer.widthTiles = width;
			layer.heightTiles = height;
			layer.transform.position[0] = x;
			layer.transform.position[1] = y;
			layer.tileWidthPx = tw;
			layer.tileHeightPx = th;
			layer.bIsObjectLayer = false;
			switch (compression)
			{
			case 1:         // RLE
				LoadTilesRLEV1(&layer, pBS);
				break;
			case 2:         // uncompressed
				LoadTilesUncompressedV1(&layer, pBS);
				break;
			default:
				Log_Error("unexpected value for compression enum %i\n", compression);
				break;
			}
			break;
		case 2: // object layer
			layer.bIsObjectLayer = true;
			BS_DeSerializeU32((u32*)&layer.drawOrder, pBS);
			Et2D_SerializeEntities(&pData->entities, pBS, pData, objectLayer++);
			break;
		default:
			EASSERT(false);
		}
		
		pTileMap->layers = VectorPush(pTileMap->layers, &layer);
	}
}

struct InitEntitiesCtx
{
	struct GameFrameworkLayer* pLayer;
	InputContext* pInputContext;
	DrawContext* pDrawContext;
};

static bool InitEntities(struct Entity2D* pEnt, int i, void* pUser)
{
	struct InitEntitiesCtx* pCtx = pUser;
	pEnt->init(pEnt, pCtx->pLayer, pCtx->pDrawContext, pCtx->pInputContext);
	return true;
}

static void LoadLevelDataInternal(struct TileMap* pTileMap, struct BinarySerializer* pBs, DrawContext* pDC, hAtlas atlas, struct GameLayer2DData* pData)
{
	u32 version = 0;
	BS_DeSerializeU32(&version, pBs);
	switch (version)
	{
	case 1:
		LoadLevelDataV1(pTileMap, pBs, pData);
		break;
	default:
		Log_Error("Unexpected tilemap file version %u\n", version);
		break;
	}
	BS_Finish(pBs);
}

static void LoadLevelDataFromServer(struct TileMap* pTileMap, DrawContext* pDC, hAtlas atlas, struct GameLayer2DData* pData)
{
	/*THIS HAS TO BE THE FIRST THING TO DEQUEUE CONNECTION EVENTS IF GAME2DLAYER IS USED*/
	EASSERT(NW_GetRole(GR_Client));
	struct NetworkConnectionEvent nce;
	while(true)
	{
		while(NW_DequeueConnectionEvent(&nce))
		{
			if(nce.type == NCE_ClientConnected)
			{
				goto connected;
			}
		}
	}
connected:
	Log_Info("game2d client connected, sending level request");
	G2D_Enqueue_RequestLevelData();
	Log_Info("Enqueued request");

	struct NetworkQueueItem nci;
	while(true)
	{
		while(NW_DequeueData(&nci))
		{
			Log_Info("Dequeued data size %i", nci.pDataSize);
			u8* pBody = NULL;
			int headerSize = 0;
			enum G2DPacketType type = G2D_ParsePacket(nci.pData, &pBody, &headerSize);
			bool bLoaded = false;
			if(type == G2DPacket_LevelDataResponseData)
			{
				struct BinarySerializer bs;
				BS_CreateForLoadFromBuffer(pBody, nci.pDataSize - headerSize, &bs);
				if(pData->levelDataHandlerExtender)
				{
					pData->levelDataHandlerExtender(pData, &bs);
				}
				LoadLevelDataInternal(pTileMap, &bs, pDC, atlas, pData);
				bLoaded = true;
			}

			free(nci.pData);
			if(bLoaded)
				goto level_loaded;
		}
	}
level_loaded:
	Log_Info("Level loaded from server"); /* because of what I can only assume is a compiler bug, there needs to be *something* after this label or msvc will chuck a syntax error here */
}

static void LoadLevelData(struct TileMap* pTileMap, const char* tilemapFilePath, DrawContext* pDC, hAtlas atlas, struct GameLayer2DData* pData)
{
	pTileMap->layers = NEW_VECTOR(struct TileMapLayer);
	struct BinarySerializer bs;
	memset(&bs, 0, sizeof(struct BinarySerializer));
	BS_CreateForLoad(tilemapFilePath, &bs);
	LoadLevelDataInternal(pTileMap, &bs, pDC, atlas, pData);
}

static void PublishDebugMessage(struct GameLayer2DData* pData)
{
	vec2 tl, br;
	GetViewportWorldspaceTLBR(tl, br, &pData->camera, pData->windowW, pData->windowH);
	sprintf(pData->debugMsg, "Tiles: %i zoom:%.2f tlx:%.2f tly:%.2f brx:%.2f bry:%.2f",
		gTilesRendered, pData->camera.scale[0],
		tl[0], tl[1],
		br[0], br[1]
	);
	struct ScriptCallArgument arg;
	arg.type = SCA_string;
	arg.val.string = pData->debugMsg;
	struct LuaListenedEventArgs args = { .numArgs = 1, .args = &arg };
	Ev_FireEvent("DebugMessage", &args);
}

struct UpdateEntityContext
{
	float deltaT;
	struct GameFrameworkLayer* pLayer;
};

struct PostPhysEntityContext
{
	struct GameFrameworkLayer* pLayer;
	float deltaT;
};

static bool PostPhysicsEntities(struct Entity2D* pEnt, int i, void* pUser)
{
	struct PostPhysEntityContext* pCtx = pUser;
	pEnt->postPhys(pEnt, pCtx->pLayer, pCtx->deltaT);
	return true;
}

static bool UpdateEntities(struct Entity2D* pEnt, int i, void* pUser)
{
	struct UpdateEntityContext* pCTX = pUser;
	pEnt->update(pEnt, pCTX->pLayer, pCTX->deltaT);
	return true;
}

void G2D_SaveLevelDataInternal(struct GameLayer2DData* pData, struct BinarySerializer* pBS)
{
	BS_SerializeU32(1, pBS);
	
	vec2 tl, br;
	vec2 dims;
	Entity2DQuadTree_GetDims(pData->hEntitiesQuadTree, tl, &dims[0], &dims[1]);
	glm_vec2_add(tl, dims, br);
	
	/* data needed to init quadtree */
	BS_SerializeFloat(tl[0], pBS);
	BS_SerializeFloat(tl[1], pBS);
	BS_SerializeFloat(br[0], pBS);
	BS_SerializeFloat(br[1], pBS);

	int numLayers = VectorSize(pData->tilemap.layers);
	BS_SerializeU32(numLayers, pBS);
	int onObjectLayer = 0;
	for(int i=0; i<numLayers; i++)
	{
		BS_SerializeU32(pData->tilemap.layers[i].type, pBS); // type of layer
		struct TileMapLayer* pLayer = &pData->tilemap.layers[i];
		switch(pLayer->type)
		{
		case 1: // tile layer
			BS_SerializeU32(pLayer->widthTiles, pBS);
			BS_SerializeU32(pLayer->heightTiles, pBS);
			BS_SerializeU32(pLayer->transform.position[1], pBS);
			BS_SerializeU32(pLayer->transform.position[1], pBS);
			BS_SerializeU32(pLayer->tileWidthPx, pBS);
			BS_SerializeU32(pLayer->tileHeightPx, pBS);
			BS_SerializeU32(2, pBS); // no compression

			// serialize the layer tiles
			BS_SerializeBytesNoLen((const char*)pLayer->Tiles, pLayer->widthTiles * pLayer->heightTiles * sizeof(TileIndex), pBS);
			break;
		case 2: // object layer
			BS_SerializeU32(pLayer->drawOrder, pBS);
			Et2D_SerializeEntities(&pData->entities, pBS, pData, onObjectLayer++);
			break;
		default:
			EASSERT(false);
		}
	}
}


static void Update(struct GameFrameworkLayer* pLayer, float deltaT)
{
	switch(NW_GetRole())
	{
	case GR_Client:
		G2D_PollNetworkQueueClient(pLayer, deltaT);
		break;
	case GR_ClientServer:
		G2D_PollNetworkQueueServer(pLayer, deltaT);
		break;
	}
	struct GameLayer2DData* pData = pLayer->userData;

	TP_DoTimers(&pData->timerPool, deltaT);
	
	if (pData->bDebugLayerAttatched)
	{
		PublishDebugMessage(pData);
	}
	struct UpdateEntityContext ctx = 
	{
		.deltaT = deltaT,
		.pLayer = pLayer
	};

	Et2D_IterateEntities(&pData->entities, &UpdateEntities, &ctx);
	Ph_PhysicsWorldStep(pData->hPhysicsWorld, deltaT, 4);
	struct PostPhysEntityContext postPhysCtx = 
	{
		.deltaT =deltaT,
		.pLayer = pLayer
	};
	Ph_PhysicsWorldDoCollisionEvents(pLayer);
	Et2D_IterateEntities(&pData->entities, &PostPhysicsEntities, &postPhysCtx);
	
	if(pData->cameraClampedToTilemapLayer >= 0)
		UpdateCameraClamp(pData);

	Et2D_DoEntityMessagesQueue(&pData->entities, pLayer);
}

void OutputSpriteVertices(
	AtlasSprite* pSprite,
	VECTOR(Worldspace2DVert)* pOutVert,
	VECTOR(VertIndexT)* pOutInd,
	VertIndexT* pNextIndex,
	int col,
	int row,
	struct Transform2D* pTMLayerTransform
)
{
	VECTOR(Worldspace2DVert) outVert = *pOutVert;
	VECTOR(VertIndexT) outInd = *pOutInd;

	VertIndexT base = *pNextIndex;
	*pNextIndex += 4;
	vec2 dims = {
		pSprite->actualWidthPX,
		pSprite->actualHeightPX
	};
	vec2 topLeft = {
		col * pSprite->widthPx,
		row * pSprite->heightPx
	};
	vec2 offset = {
		pSprite->xOffsetToActual,
		pSprite->yOffsetToActual
	};
	glm_vec2_add(topLeft, offset, topLeft);

	glm_vec2_add(pTMLayerTransform->position, topLeft, topLeft);
	vec2 bottomRight;
	glm_vec2_add(topLeft, dims, bottomRight);

	vec2 topRight = {
		topLeft[0] + pSprite->actualWidthPX,
		topLeft[1]
	};

	vec2 bottomLeft = {
		topLeft[0],
		topLeft[1] + pSprite->actualHeightPX
	};
	
	Worldspace2DVert vert = {
		topLeft[0], topLeft[1],
		pSprite->topLeftUV_U, pSprite->topLeftUV_V
	};

	// top left
	VertIndexT tl = base;
	outVert = VectorPush(outVert, &vert);
	
	vert.x = topRight[0]; 
	vert.y = topRight[1];
	vert.u = pSprite->bottomRightUV_U;
	vert.v = pSprite->topLeftUV_V;
	
	// top right
	VertIndexT tr = base + 1;
	outVert = VectorPush(outVert, &vert);

	vert.x = bottomLeft[0];
	vert.y = bottomLeft[1];
	vert.u = pSprite->topLeftUV_U;
	vert.v = pSprite->bottomRightUV_V;

	// bottom left
	VertIndexT bl = base + 2;
	outVert = VectorPush(outVert, &vert);

	vert.x = bottomRight[0];
	vert.y = bottomRight[1];
	vert.u = pSprite->bottomRightUV_U;
	vert.v = pSprite->bottomRightUV_V;

	// bottom right
	VertIndexT br = base + 3;
	outVert = VectorPush(outVert, &vert);

	outInd = VectorPush(outInd, &tl);
	outInd = VectorPush(outInd, &tr);
	outInd = VectorPush(outInd, &bl);
	outInd = VectorPush(outInd, &tr);
	outInd = VectorPush(outInd, &br);
	outInd = VectorPush(outInd, &bl);

	*pOutVert = outVert;
	*pOutInd = outInd;
}

static void OutputTilemapLayerVertices(
	hAtlas atlas,
	struct TileMapLayer* pLayer,
	VECTOR(Worldspace2DVert)* outVerts,
	VECTOR(VertIndexT)* outInds,
	VertIndexT* pNextIndex,
	vec2 viewportTL,
	vec2 viewportBR
)
{
	VECTOR(Worldspace2DVert) outVert = *outVerts;
	VECTOR(VertIndexT) outInd = *outInds;

	/*
		Only draw those tiles that are in the viewport:
		TODO: make this work for layers that are transformed
	*/

	int startCol = ((int)viewportTL[0]) / pLayer->tileWidthPx;
	startCol = startCol < 0 ? 0 : startCol;
	int endCol = ((int)viewportBR[0]) / pLayer->tileWidthPx;
	endCol++;
	endCol = endCol > pLayer->widthTiles ? pLayer->widthTiles : endCol;


	int startRow = ((int)viewportTL[1]) / pLayer->tileHeightPx;
	startRow = startRow < 0 ? 0 : startRow;
	int endRow = ((int)viewportBR[1]) / pLayer->tileHeightPx;
	endRow++;
	endRow = endRow > pLayer->heightTiles ? pLayer->heightTiles : endRow;

	for (int row = startRow; row < endRow; row++)
	{
		for (int col = startCol; col < endCol; col++)
		{
			TileIndex tile = pLayer->Tiles[row * pLayer->widthTiles + col];
			if (tile == 0)
			{
				continue;
			}
			hSprite sprite = At_TilemapIndexToSprite(atlas, tile);
			AtlasSprite* pSprite = At_GetSprite(sprite, atlas);
			OutputSpriteVertices(pSprite, &outVert, &outInd, pNextIndex, col, row, &pLayer->transform);
			gTilesRendered++;
		}
	}

	*outVerts = outVert;
	*outInds = outInd;
}

/* Hack */
static struct Entity2DCollection* gEntityCollectionCurrent = NULL;

static int EntityDrawOrderCompare(const void* a, const void* b)
{
	HEntity2D entA = *((HEntity2D*)a);
	HEntity2D entB = *((HEntity2D*)b);
	struct Entity2D* pEntA = Et2D_GetEntity(gEntityCollectionCurrent, entA);
	struct Entity2D* pEntB = Et2D_GetEntity(gEntityCollectionCurrent, entB);

	float aval = pEntA->getSortPos(pEntA);
	float bval =  pEntB->getSortPos(pEntB);
	if(CompareFloat(aval, bval))
	{
		return 0;
	}
	if(aval < bval)
	{
		return -1; /* place first argument before second */
	}
	return 1;      /* place second argument before first */
}

VECTOR(HEntity2D) Et2D_QueryDynEntities(struct GameFrameworkLayer* pLayer, struct Entity2DCollection* pCollection, vec2 regionTL, vec2 regionBR, VECTOR(HEntity2D) pOutEntities)
{
	struct DynamicEnt2DList* pList = &pCollection->dynamicEntities;
	HDynamicEntityListItem hOn = NULL_HANDLE;
	hOn = pList->hDynamicListHead;
	while(hOn != NULL_HANDLE)
	{
		struct DynamicEntityListItem* pItem = &pList->pDynamicListItemPool[hOn];
		struct Entity2D* pEnt = Et2D_GetEntity(pCollection, pItem->hEnt);
		vec2 entTL, entBR;
		pEnt->getBB(pEnt, pLayer, entTL, entBR);
		if(Ge_AABBIntersect(regionTL, regionBR, entTL, entBR))
		{
			pOutEntities = VectorPush(pOutEntities, &pItem->hEnt);
		}
		hOn = pItem->hNext;
	}
	return pOutEntities;
} 

/* TODO: move somewhere sensible */
bool gDrawDebugLines = true;

static void OutputVertices(
	struct TileMap* pData, 
	struct Transform2D* pCam, 
	VECTOR(Worldspace2DVert)* outVerts,
	VECTOR(VertIndexT)* outIndices,

	VECTOR(WorldspaceLineVertex)* pOutLines,

	struct GameLayer2DData* pLayerData,
	struct GameFrameworkLayer* pLayer
)
{
	VECTOR(Worldspace2DVert) verts = *outVerts;
	VECTOR(VertIndexT) inds = *outIndices;
	static VECTOR(HEntity2D) sFoundEnts = NULL;
	if(!sFoundEnts)
	{
		sFoundEnts = NEW_VECTOR(HEntity2D);
	}
	sFoundEnts = VectorClear(sFoundEnts);
	int foundEnts = VectorSize(sFoundEnts);

	vec2 tl, br;
	GetViewportWorldspaceTLBR(tl, br, pCam, pLayerData->windowW, pLayerData->windowH);
	/* query the quadtree for entities here */
	sFoundEnts = Entity2DQuadTree_Query(pLayerData->hEntitiesQuadTree, tl, br, sFoundEnts, &pLayerData->entities, pLayer);
	/* query dynamic entities */
	sFoundEnts = Et2D_QueryDynEntities(pLayer, &pLayerData->entities, tl, br, sFoundEnts);
	/* sort the entities */
	gEntityCollectionCurrent = &pLayerData->entities;
	foundEnts = VectorSize(sFoundEnts);

	qsort(sFoundEnts, foundEnts, sizeof(HEntity2D), &EntityDrawOrderCompare);
	gTilesRendered = 0;
	VertIndexT nextIndexVal = 0;
	int onObjectLayer = 0;
	for (int i = 0; i < VectorSize(pData->layers); i++)
	{
		if(pData->layers[i].bIsObjectLayer)
		{
			/* from the entities we've found from the quad tree, draw the ones that are in this layer */
			for(int j=0; j<VectorSize(sFoundEnts); j++)
			{
				struct Entity2D* pEnt = Et2D_GetEntity(&pLayerData->entities, sFoundEnts[j]);
				if(onObjectLayer == pEnt->inDrawLayer)
				{
					pEnt->draw(pEnt, pLayer, &pEnt->transform, &verts, &inds, &nextIndexVal);
					if(gDrawDebugLines && pEnt->drawDebugLines)
					{
						pEnt->drawDebugLines(pEnt, pLayer, pCam, &pLayerData->pWorldspaceLineVertices);
					}
				}
			}
			onObjectLayer++;
		}
		else
		{
			OutputTilemapLayerVertices(pLayerData->hAtlas, pData->layers + i, &verts, &inds, &nextIndexVal, tl, br);
		}
	}

	*outVerts = verts;
	*outIndices = inds;
}

static void Draw(struct GameFrameworkLayer* pLayer, DrawContext* context)
{
	struct GameLayer2DData* pData = pLayer->userData;

	if(pData->bSkipDraw)
	{
		pData->bSkipDraw = false;
		return;
	}
	At_SetCurrent(pData->hAtlas, context);
	pData->pWorldspaceVertices = VectorClear(pData->pWorldspaceVertices);
	pData->pWorldspaceIndices = VectorClear(pData->pWorldspaceIndices);
	pData->pWorldspaceLineVertices = VectorClear(pData->pWorldspaceLineVertices);
	OutputVertices(&pData->tilemap, &pData->camera, &pData->pWorldspaceVertices, &pData->pWorldspaceIndices, &pData->pWorldspaceLineVertices, pData, pLayer);
	context->WorldspaceVertexBufferData(pData->vertexBuffer, pData->pWorldspaceVertices, VectorSize(pData->pWorldspaceVertices), pData->pWorldspaceIndices, VectorSize(pData->pWorldspaceIndices));
	context->WorldspaceLineBufferData(pData->lineBuffer, pData->pWorldspaceLineVertices, VectorSize(pData->pWorldspaceLineVertices));
	mat4 view;
	glm_mat4_identity(view);
	
	vec3 translate = {
		pData->camera.position[0],
		pData->camera.position[1],
		0.0f
	};
	vec3 scale = {
		pData->camera.scale[0],
		pData->camera.scale[1],
		1.0f
	};

	glm_scale(view, scale);
	glm_translate(view, translate);

	context->DrawWorldspaceVertexBuffer(pData->vertexBuffer, VectorSize(pData->pWorldspaceIndices), view);
	context->DrawWorldspaceLineVertexBuffer(pData->lineBuffer, VectorSize(pData->pWorldspaceLineVertices), view);
}


struct InputEntityContext
{
	struct GameFrameworkLayer* pLayer;
	InputContext* inputCtx;
};

static bool InputEntities(struct Entity2D* pEnt, int i, void* pUser)
{
	struct InputEntityContext* pCtx = pUser;
	pEnt->input(pEnt, pCtx->pLayer, pCtx->inputCtx);
	return true;
}

static void Input(struct GameFrameworkLayer* pLayer, InputContext* context)
{
	struct GameLayer2DData* pData = pLayer->userData;
	if (pData->bDebugLayerAttatched)
	{
		FreeLookMode2DInput(pLayer, context);
	}
	struct InputEntityContext ctx =
	{
		.pLayer = pLayer,
		.inputCtx = context
	};
	Et2D_IterateEntities(&pData->entities, &InputEntities, &ctx);
}

static void LoadLayerAssets(struct GameLayer2DData* pData, DrawContext* pDC)
{
	struct BinarySerializer bs;
	memset(&bs, 0, sizeof(struct BinarySerializer));
	BS_CreateForLoad(pData->atlasFilePath, &bs);
	At_SerializeAtlas(&bs, &pData->hAtlas, pDC);
	BS_Finish(&bs);
	if(pData->preLoadLevelFn)
	{
		pData->preLoadLevelFn(pData);
	}
	if(NW_GetRole() == GR_Client)
	{
		LoadLevelDataFromServer(&pData->tilemap, pDC, pData->hAtlas, pData);
	}
	else
	{
		LoadLevelData(&pData->tilemap, pData->tilemapFilePath, pDC, pData->hAtlas, pData);
	}
	
	pData->bLoaded = true;
}


static void ActivateFreeLookMode(InputContext* inputContext, struct GameLayer2DData* pData)
{
	In_SetMask(&pData->freeLookCtrls.freeLookInputsMask, inputContext);
}

static void OnDebugLayerPushed(void* pUserData, void* pEventData)
{
	struct GameLayer2DData* pData = pUserData;
	pData->bDebugLayerAttatched = true;
}
 
static bool NetworkUpdateWorldstateCallback(struct SDTimer* pTimer)
{
	switch(NW_GetRole())
	{
	case GR_Client:
		{
			G2D_Enqueue_Worldstate_Packet(pTimer->pUserData, -1);
		}
		break;
	case GR_ClientServer:
		{
			for(int i=0; i<3; i++)
			{
				if(G2D_IsClientConnected(i))
				{
					G2D_Enqueue_Worldstate_Packet(pTimer->pUserData, i);
				}
			}
		}
		break;
	}
	return false;
}

void GameLayer2D_OnPush(struct GameFrameworkLayer* pLayer, DrawContext* drawContext, InputContext* inputContext)
{
	struct GameLayer2DData* pData = pLayer->userData;
	pData->pLayer = pLayer;
	Et2D_InitCollection(&pData->entities);
	pData->hPhysicsWorld = Ph_GetPhysicsWorld(0, 0, 32.0f); // todo - pass these arguments in somehow
	BindFreeLookControls(inputContext, pData);
	ActivateFreeLookMode(inputContext, pData);
	if (!pData->bLoaded)
	{
		LoadLayerAssets(pData, drawContext);
	}
	if(pData->preFirstInitCallback)
		pData->preFirstInitCallback(pData);
	struct InitEntitiesCtx ctx = {
		.pDrawContext = drawContext,
		.pInputContext = inputContext,
		.pLayer = pLayer
	};
	Et2D_IterateEntities(&pData->entities, &InitEntities, &ctx);

	pData->pDebugListener = Ev_SubscribeEvent("onDebugLayerPushed", &OnDebugLayerPushed, pData);

	/*
		HACK:
		todo sort out the availabilty of these draw and input contexts
	*/
	pData->pDrawContext = drawContext;

	if(NW_GetRole() != GR_Singleplayer)
	{
		struct SDTimer timerDef = {
			.bActive = true,
			.bRepeat = true,
			.bAutoReset = true,
			.total = 0.1f,
			.fnCallback = &NetworkUpdateWorldstateCallback,
			.pUserData = pData,
			.elapsed = 0.0
		};
		pData->hNetworkStateUpdateTimer = TP_GetTimer(&pData->timerPool, &timerDef);
	}
}

void Game2DLayer_OnPop(struct GameFrameworkLayer* pLayer, DrawContext* drawContext, InputContext* inputContext)
{
	struct GameLayer2DData* pData = pLayer->userData;
	EASSERT(pData->pDebugListener);
	Et2D_DestroyCollection(&pData->entities, pLayer);
	Ev_UnsubscribeEvent(pData->pDebugListener);
	Ph_DestroyPhysicsWorld(pData->hPhysicsWorld);
	drawContext->DestroyWorldspaceVertexBuffer(pData->vertexBuffer);
	drawContext->DestroyWorldspaceLineVertexBuffer(pData->lineBuffer);
	DestoryVector(pData->pWorldspaceVertices);
	DestoryVector(pData->pWorldspaceIndices);
	DestoryVector(pData->pWorldspaceLineVertices);
}

static void OnWindowDimsChange(struct GameFrameworkLayer* pLayer, int newW, int newH)
{
	struct GameLayer2DData* pData = pLayer->userData;
	pData->windowW = newW;
	pData->windowH = newH;
}

void Game2DLayer_Get(struct GameFrameworkLayer* pLayer, struct Game2DLayerOptions* pOptions, DrawContext* pDC)
{
	pLayer->userData = malloc(sizeof(struct GameLayer2DData));
	memset(pLayer->userData, 0, sizeof(struct GameLayer2DData));
	struct GameLayer2DData* pData = pLayer->userData;

	pData->bSkipDraw = true;

	pData->cameraClampedToTilemapLayer = -1;
	pData->tilemap.layers = NEW_VECTOR(struct TileMapLayer);

	EASSERT(strlen(pData->tilemapFilePath) < 128);
	EASSERT(strlen(pData->atlasFilePath) < 128);
	strcpy(pData->tilemapFilePath, pOptions->levelFilePath);
	strcpy(pData->atlasFilePath, pOptions->atlasFilePath);

	pLayer->update = &Update;
	pLayer->draw = &Draw;
	pLayer->input = &Input;
	pLayer->onPush = &GameLayer2D_OnPush;
	pLayer->onPop = &Game2DLayer_OnPop;
	pLayer->onWindowDimsChanged = &OnWindowDimsChange;
	
	pData->hNetworkStateUpdateTimer = NULL_HANDLE;

	pData->camera.scale[0] = 1;
	pData->camera.scale[1] = 1;

	pData->vertexBuffer = pDC->NewWorldspaceVertBuffer(256);
	pData->lineBuffer = pDC->NewWorldspaceLineBuffer(256);
	
	pData->pWorldspaceVertices = NEW_VECTOR(Worldspace2DVert);
	pData->pWorldspaceIndices = NEW_VECTOR(VertIndexT);
	pData->pWorldspaceLineVertices = NEW_VECTOR(WorldspaceLineVertex);

	pData->windowH = pDC->screenHeight;
	pData->windowW = pDC->screenWidth;

	pData->bCurrentLocationIsDirty = false;

	pData->timerPool = TP_InitTimerPool(16);
}

void Game2DLayer_SaveLevelFile(struct GameLayer2DData* pData, const char* outputFilePath)
{
	struct BinarySerializer bs;
	memset(&bs, 0, sizeof(struct BinarySerializer));
	BS_CreateForSave(outputFilePath, &bs);
	G2D_SaveLevelDataInternal(pData, &bs);
	BS_Finish(&bs);
}


void Et2D_Transform2DToMat3(struct Transform2D* pTrans, mat3 out)
{
    mat3 scale = 
    {
        {pTrans->scale[0],                0, 0},
        {               0, pTrans->scale[1], 0},
        {               0,                0, 1}

    };
    mat3 trans = 
    {
        {                  1,                   0, 0},
        {                  0,                   1, 0},
        {pTrans->position[0], pTrans->position[1], 1}
    };
	// float sinT = sin(pTrans->rotation);
    // float cosT = cos(pTrans->rotation);
    // vec2 c;
    // glm_vec2_add(pTrans->position, pTrans->rotationPointRelative, c);
    // mat3 rot = 
    // {
    //     {                           cosT,                            sinT, 0},
    //     {                          -sinT,                            cosT, 0},
    //     {c[0] * (1 - cosT) + c[1] * sinT, c[1] * (1 - cosT) - c[0] * sinT, 1}
    // };
    //glm_mat3_mul(scale, rot, out);
    glm_mat3_mul(trans, scale, out);
}