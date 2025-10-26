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
				printf("unexpected value for compression enum %i\n", compression);
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

static void LoadLevelData(struct TileMap* pTileMap, const char* tilemapFilePath, DrawContext* pDC, hAtlas atlas, struct GameLayer2DData* pData)
{
	pTileMap->layers = NEW_VECTOR(struct TileMapLayer);
	struct BinarySerializer bs;
	memset(&bs, 0, sizeof(struct BinarySerializer));
	BS_CreateForLoad(tilemapFilePath, &bs);
	u32 version = 0;
	BS_DeSerializeU32(&version, &bs);
	switch (version)
	{
	case 1:
		LoadLevelDataV1(pTileMap, &bs, pData);
		break;
	default:
		printf("Unexpected tilemap file version %u\n", version);
		break;
	}
	BS_Finish(&bs);

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

static void Update(struct GameFrameworkLayer* pLayer, float deltaT)
{
	struct GameLayer2DData* pData = pLayer->userData;
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
		pSprite->widthPx,
		pSprite->heightPx
	};
	vec2 topLeft = {
		col * pSprite->widthPx,
		row * pSprite->heightPx
	};

	glm_vec2_add(pTMLayerTransform->position, topLeft, topLeft);
	vec2 bottomRight;
	glm_vec2_add(topLeft, dims, bottomRight);

	vec2 topRight = {
		topLeft[0] + pSprite->widthPx,
		topLeft[1]
	};

	vec2 bottomLeft = {
		topLeft[0],
		topLeft[1] + pSprite->heightPx
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

static VECTOR(HEntity2D) QueryVisibleDynEntities(struct GameFrameworkLayer* pLayer, struct Entity2DCollection* pCollection, vec2 viewportTL, vec2 viewportBR, VECTOR(HEntity2D) pOutEntities)
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
		if(Ge_AABBIntersect(viewportTL, viewportBR, entTL, entBR))
		{
			pOutEntities = VectorPush(pOutEntities, &pItem->hEnt);
		}
		hOn = pItem->hNext;
	}
	return pOutEntities;
} 

static void OutputVertices(
	struct TileMap* pData, 
	struct Transform2D* pCam, 
	VECTOR(Worldspace2DVert)* outVerts,
	VECTOR(VertIndexT)* outIndices,
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
	sFoundEnts = QueryVisibleDynEntities(pLayer, &pLayerData->entities, tl, br, sFoundEnts);
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
	At_SetCurrent(pData->hAtlas, context);
	pData->pWorldspaceVertices = VectorClear(pData->pWorldspaceVertices);
	pData->pWorldspaceIndices = VectorClear(pData->pWorldspaceIndices);
	OutputVertices(&pData->tilemap, &pData->camera, &pData->pWorldspaceVertices, &pData->pWorldspaceIndices, pData, pLayer);
	context->WorldspaceVertexBufferData(pData->vertexBuffer, pData->pWorldspaceVertices, VectorSize(pData->pWorldspaceVertices), pData->pWorldspaceIndices, VectorSize(pData->pWorldspaceIndices));
	mat4 view;
	glm_mat4_identity(view);
	// TODO: set here based on camera
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
	LoadLevelData(&pData->tilemap, pData->tilemapFilePath, pDC, pData->hAtlas, pData);
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

void GameLayer2D_OnPush(struct GameFrameworkLayer* pLayer, DrawContext* drawContext, InputContext* inputContext)
{
	struct GameLayer2DData* pData = pLayer->userData;
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
	//XMLUI_PushGameFrameworkLayer("./Assets/debug_overlay.xml");

	/*
		HACK:
		todo sort out the availabilty of these draw and input contexts
	*/
	pData->pDrawContext = drawContext;
}

void Game2DLayer_OnPop(struct GameFrameworkLayer* pLayer, DrawContext* drawContext, InputContext* inputContext)
{
	struct GameLayer2DData* pData = pLayer->userData;
	EASSERT(pData->pDebugListener);
	Et2D_DestroyCollection(&pData->entities);
	Ev_UnsubscribeEvent(pData->pDebugListener);
	Ph_DestroyPhysicsWorld(pData->hPhysicsWorld);
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

	pData->pLayer = pLayer;
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

	pData->camera.scale[0] = 1;
	pData->camera.scale[1] = 1;

	pData->vertexBuffer = pDC->NewWorldspaceVertBuffer(256);
	pData->pWorldspaceVertices = NEW_VECTOR(Worldspace2DVert);
	pData->pWorldspaceIndices = NEW_VECTOR(VertIndexT);

	pData->windowH = pDC->screenHeight;
	pData->windowW = pDC->screenWidth;
}

void Game2DLayer_SaveLevelFile(struct GameLayer2DData* pData, const char* outputFilePath)
{
	struct BinarySerializer bs;
	memset(&bs, 0, sizeof(struct BinarySerializer));
	BS_CreateForSave(outputFilePath, &bs);
	BS_SerializeU32(1, &bs);

	vec2 tl, br;
	vec3 dims;
	glm_vec2_add(tl, dims, br);
	Entity2DQuadTree_GetDims(pData->hEntitiesQuadTree, tl, &dims[0], &dims[1]);
	
	
	/* data needed to init quadtree */
	BS_SerializeFloat(tl[0], &bs);
	BS_SerializeFloat(tl[1], &bs);
	BS_SerializeFloat(br[0], &bs);
	BS_SerializeFloat(br[1], &bs);

	int numLayers = VectorSize(pData->tilemap.layers);
	BS_SerializeU32(numLayers, &bs);
	int onObjectLayer = 0;
	for(int i=0; i<numLayers; i++)
	{
		BS_SerializeU32(pData->tilemap.layers[i].type, &bs); // type of layer
		struct TileMapLayer* pLayer = &pData->tilemap.layers[i];
		switch(pLayer->type)
		{
		case 1: // tile layer
			BS_SerializeU32(pLayer->widthTiles, &bs);
			BS_SerializeU32(&pLayer->heightTiles, &bs);
			BS_SerializeU32(&pLayer->transform.position[1], &bs);
			BS_SerializeU32(&pLayer->transform.position[1], &bs);
			BS_SerializeU32(&pLayer->tileWidthPx, &bs);
			BS_SerializeU32(&pLayer->tileWidthPx, &bs);
			BS_SerializeU32(2, &bs); // no compression

			// serialize the layer tiles
			BS_SerializeBytes(pLayer->Tiles, pLayer->widthTiles * pLayer->heightTiles * sizeof(TileIndex), &bs);
			break;
		case 2: // object layer
			BS_SerializeU32(pLayer->drawOrder, &bs);
			Et2D_SerializeEntities(&pData->entities, &bs, pData, onObjectLayer++);
			break;
		default:
			EASSERT(false);
		}
	}
}
