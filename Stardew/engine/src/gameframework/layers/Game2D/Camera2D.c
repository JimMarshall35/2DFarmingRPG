#include "Camera2D.h"
#include "Game2DLayer.h"
#include "AssertLib.h"

void GetViewportWorldspaceTLBR(vec2 outTL, vec2 outBR, struct Transform2D* pCam, int windowW, int windowH)
{
	outTL[0] = pCam->position[0];
	outTL[1] = pCam->position[1];

	outTL[0] = -outTL[0];
	outTL[1] = -outTL[1];

	outBR[0] = outTL[0] + windowW / pCam->scale[0];
	outBR[1] = outTL[1] + windowH / pCam->scale[0];

}

void CenterCameraAt(float worldspaceX, float worldspaceY, struct Transform2D* pCam, int winWidth, int winHeight)
{
	vec2 tl, br;
	GetViewportWorldspaceTLBR(tl, br, pCam, winWidth, winHeight);
	pCam->position[0] = -(worldspaceX - (br[0] - tl[0]) / 2.0f);
	pCam->position[1] = -(worldspaceY - (br[1] - tl[1]) / 2.0f);
}

void GetCamWorldspaceCenter(struct Transform2D* pCam, int winWidth, int winHeight, vec2 outCenter)
{
	vec2 tl, br;
	vec2 add;
	GetViewportWorldspaceTLBR(tl, br, pCam, winWidth, winHeight);
	add[0] = (br[0] - tl[0]) / 2.0f;
	add[1] = (br[1] - tl[1]) / 2.0f;
	glm_vec2_add(tl, add, outCenter);
}

void GetWorldspacePos(int screenPosX, int screenPosY, int screenW, int screenH, struct Transform2D* pCam, vec2 outWorldspace)
{
	vec2 tl, br;
	GetViewportWorldspaceTLBR(tl, br, pCam, screenW, screenH);
	float fX = (float)screenPosX / (float)screenW;
	float fY = (float)screenPosY / (float)screenH;
	outWorldspace[0] = tl[0] + fX * (br[0] - tl[0]);
	outWorldspace[1] = tl[1] + fY * (br[1] - tl[1]);
}

void ScreenSpaceToWorldSpaceTransVector(vec2 screenSpaceTranslateVector, int screenW, int screenH, struct Transform2D* pCam, vec2 outWorldspace)
{
	vec2 tl, br;
	GetViewportWorldspaceTLBR(tl, br, pCam, screenW, screenH);
	vec2 norm = {
		screenSpaceTranslateVector[0] / screenW,
		screenSpaceTranslateVector[1] / screenH
	};
	outWorldspace[0] = norm[0] * (br[0] - tl[0]);
	outWorldspace[1] = norm[1] * (br[1] - tl[1]);
}

void ClampCameraToTileLayer(struct GameLayer2DData* pGameLayerData, int tileLayerNum)
{
	EASSERT(tileLayerNum >= 0 && tileLayerNum < VectorSize(pGameLayerData->tilemap.layers));
	pGameLayerData->cameraClampedToTilemapLayer = tileLayerNum;
}

void UpdateCameraClamp(struct GameLayer2DData* pGameLayerData)
{
	
	if(pGameLayerData->cameraClampedToTilemapLayer < 0)
	{
		return;
	}
	struct Transform2D* pCam = &pGameLayerData->camera;
	struct TileMapLayer* pTilemapLayer = &pGameLayerData->tilemap.layers[pGameLayerData->cameraClampedToTilemapLayer];
	vec2 tl = {
		pTilemapLayer->transform.position[0],
		pTilemapLayer->transform.position[1]
	};
	vec2 dims = {
		pTilemapLayer->widthTiles * pTilemapLayer->tileWidthPx,
		pTilemapLayer->heightTiles * pTilemapLayer->tileHeightPx,
	};
	vec2 br;
	glm_vec2_add(tl, dims, br);
	vec2 camTL, camBR;
	GetViewportWorldspaceTLBR(camTL, camBR, pCam, pGameLayerData->windowW, pGameLayerData->windowH);

	float tilemapW = br[0] - tl[0];
	float tilemapH = br[1] - tl[1];
	float viewportW = camBR[0] - camTL[0];
	float viewportH = camBR[1] - camTL[1];

	vec2 camCenter;
	GetCamWorldspaceCenter(pCam, pGameLayerData->windowW, pGameLayerData->windowH, camCenter);


	float xAdjust = 0.0f;
	float yAdjust = 0.0f;
	if(camTL[0] < tl[0])
	{
		xAdjust = tl[0] - camTL[0];
	}
	else if(camBR[0] > br[0])
	{
		xAdjust = br[0] - camBR[0];
	}

	if(camTL[1] < tl[1])
	{
		yAdjust = tl[1] - camTL[1];
	}
	else if(camBR[1] > br[1])
	{
		yAdjust = br[1] - camBR[1];
	}

	vec2 tmCentre; /* center of the tilemap in world space */
	vec2 tmHalfDims = 
	{
		dims[0] / 2.0f,
		dims[1] / 2.0f
	};
	glm_vec2_add(tl, tmHalfDims, tmCentre);

	/* if the tilemap is smaller than the viewport then cemtre the camera at the centre of the tilemap */
	if(tilemapW <= viewportW)
	{
		vec2 camCenter = {
			tmCentre[0],
			camCenter[1]
		};
		xAdjust = 0.0f;
		CenterCameraAt(camCenter[0], camCenter[1], pCam, pGameLayerData->windowW, pGameLayerData->windowH);
	}
	if(tilemapH <= viewportH)
	{
		vec2 camCenter = {
			camCenter[0],
			tmCentre[1]
		};
		yAdjust = 0.0f;
		CenterCameraAt(camCenter[0], camCenter[1], pCam, pGameLayerData->windowW, pGameLayerData->windowH);
	}


	pCam->position[0] -= xAdjust;
	pCam->position[1] -= yAdjust;
}
 