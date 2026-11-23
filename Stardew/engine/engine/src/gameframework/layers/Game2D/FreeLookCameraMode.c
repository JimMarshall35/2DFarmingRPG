#include "FreeLookCameraMode.h"
#include "GameFramework.h"
#include <cglm/cglm.h>
#include "Game2DLayer.h"
#include "Camera2D.h"
#define FREE_LOOK_ZOOM_IN_BINDING_NAME "zoomIn"
#define FREE_LOOK_ZOOM_OUT_BINDING_NAME "zoomOut"
#define FREE_LOOK_XPOS_BINDING_NAME "moveRight"
#define FREE_LOOK_XNEG_BINDING_NAME "moveLeft"
#define FREE_LOOK_YPOS_BINDING_NAME "moveDown"
#define FREE_LOOK_YNEG_BINDING_NAME "moveUp"
#define FREE_LOOK_CURSOR_X_BINDING_NAME "CursorPosX"
#define FREE_LOOK_CURSOR_Y_BINDING_NAME "CursorPosY"
#define FREE_LOOK_CURSOR_SELECT_BINDING_NAME "select"

float gDebugWSX = 0;
float gDebugWSY = 0;

void FreeLookMode2DInput(struct GameFrameworkLayer* pLayer, InputContext* context)
{
    struct GameLayer2DData* pData = pLayer->userData;

	if(In_GetButtonPressThisFrame(context, pData->freeLookCtrls.exitFreeLookModeBinding))
	{
		GF_PopGameFrameworkLayer();
		return;
	}

	const float FREE_LOOK_ZOOM_FACTOR = 0.99;
	const float FREE_LOOK_MOVEMENT_SPEED = 0.3;
	bool bZoomIn = In_GetButtonValue(context, pData->freeLookCtrls.freeLookZoomInBinding);
	bool bZoomOut = In_GetButtonValue(context, pData->freeLookCtrls.freeLookZoomOutBinding);
	bool bXPos = In_GetButtonValue(context, pData->freeLookCtrls.freeLookZoomMoveXPosBinding);
	bool bXNeg = In_GetButtonValue(context, pData->freeLookCtrls.freeLookZoomMoveXNegBinding);

	bool bYPos = In_GetButtonValue(context, pData->freeLookCtrls.freeLookZoomMoveYPosBinding);
	bool bYNeg = In_GetButtonValue(context, pData->freeLookCtrls.freeLookZoomMoveYNegBinding);

	vec2 movementVector = { 0,0 };
	if (bXPos)
	{
		vec2 dir = { 1,0 };
		glm_vec2_add(movementVector, dir, movementVector);
	}
	if (bXNeg)
	{
		vec2 dir = { -1,0 };
		glm_vec2_add(movementVector, dir, movementVector);
	}
	if (bYPos)
	{
		vec2 dir = { 0,-1 };
		glm_vec2_add(movementVector, dir, movementVector);
	}
	if (bYNeg)
	{
		vec2 dir = { 0,1 };
		glm_vec2_add(movementVector, dir, movementVector);
	}
	glm_vec2_normalize(movementVector);
	movementVector[0] = movementVector[0] * FREE_LOOK_MOVEMENT_SPEED;
	movementVector[1] = movementVector[1] * FREE_LOOK_MOVEMENT_SPEED;

	glm_vec2_add(movementVector, pData->camera.position, pData->camera.position);

    vec2 camcenter;
    GetCamWorldspaceCenter(&pData->camera, pData->windowW, pData->windowH, camcenter);
	if (bZoomOut)
	{
		pData->camera.scale[0] -= 0.1f;
		pData->camera.scale[1] -= 0.1f;
        CenterCameraAt(camcenter[0], camcenter[1], &pData->camera, pData->windowW, pData->windowH);
	}
	if (bZoomIn)
	{
		pData->camera.scale[0] += 0.1f;
		pData->camera.scale[1] += 0.1f;
        CenterCameraAt(camcenter[0], camcenter[1], &pData->camera, pData->windowW, pData->windowH);
	}
    

	// move around with mouse
	bool bSelectVal = In_GetButtonValue(context, pData->freeLookCtrls.freeLookSelectButtonBinding);
	static bool bLastSelect = false;
	static ivec2 dragAnchorScreenSpace;
	static vec2 dragAnchorWorldSpace;
	if(bSelectVal && !bLastSelect)
	{
		//press 
		int screenX = In_GetAxisValue(context, pData->freeLookCtrls.freeLookCursorXAxisBinding);
		int screenY = In_GetAxisValue(context, pData->freeLookCtrls.freeLookCursorYAxisBinding);
		GetWorldspacePos(screenX, screenY, pData->windowW, pData->windowH, &pData->camera, dragAnchorWorldSpace);
		gDebugWSX = dragAnchorWorldSpace[0];
		gDebugWSY = dragAnchorWorldSpace[1];
		dragAnchorScreenSpace[0] = screenX;
		dragAnchorScreenSpace[1] = screenY;
		//CenterCameraAt(gDebugWSX, gDebugWSY, &pData->camera, pData->windowW, pData->windowH); // to test
		GetCamWorldspaceCenter(&pData->camera, pData->windowW, pData->windowH, dragAnchorWorldSpace);
	}
	if(bSelectVal)
	{
		// drag	
		ivec2 ss;
		ivec2 sstrans;
		vec2 sstransf;
		vec2 wstrans;
		vec2 camCenter;
		ss[0] = In_GetAxisValue(context, pData->freeLookCtrls.freeLookCursorXAxisBinding);
		ss[1] = In_GetAxisValue(context, pData->freeLookCtrls.freeLookCursorYAxisBinding);
		glm_ivec2_sub(ss, dragAnchorScreenSpace, sstrans);
		sstransf[0] = (float)sstrans[0];
		sstransf[1] = (float)sstrans[1];
		ScreenSpaceToWorldSpaceTransVector(sstransf, pData->windowW, pData->windowH, &pData->camera, wstrans);
		glm_vec2_scale(wstrans, -1.0f, wstrans);
		glm_vec2_add(dragAnchorWorldSpace, wstrans, camCenter);
		CenterCameraAt(camCenter[0], camCenter[1], &pData->camera, pData->windowW, pData->windowH); // to test
		
	}

	bLastSelect = bSelectVal;
}

void BindFreeLookControls(InputContext* inputContext, struct GameLayer2DData* pData)
{
	pData->freeLookCtrls.freeLookZoomInBinding = In_FindButtonMapping(inputContext, FREE_LOOK_ZOOM_IN_BINDING_NAME);
	pData->freeLookCtrls.freeLookZoomOutBinding = In_FindButtonMapping(inputContext, FREE_LOOK_ZOOM_OUT_BINDING_NAME);
	pData->freeLookCtrls.freeLookZoomMoveXPosBinding = In_FindButtonMapping(inputContext, FREE_LOOK_XPOS_BINDING_NAME);
	pData->freeLookCtrls.freeLookZoomMoveXNegBinding = In_FindButtonMapping(inputContext, FREE_LOOK_XNEG_BINDING_NAME);
	pData->freeLookCtrls.freeLookZoomMoveYPosBinding = In_FindButtonMapping(inputContext, FREE_LOOK_YPOS_BINDING_NAME);
	pData->freeLookCtrls.freeLookZoomMoveYNegBinding = In_FindButtonMapping(inputContext, FREE_LOOK_YNEG_BINDING_NAME);

	pData->freeLookCtrls.freeLookCursorXAxisBinding = In_FindAxisMapping(inputContext, FREE_LOOK_CURSOR_X_BINDING_NAME);
	pData->freeLookCtrls.freeLookCursorYAxisBinding = In_FindAxisMapping(inputContext, FREE_LOOK_CURSOR_Y_BINDING_NAME);
	pData->freeLookCtrls.freeLookSelectButtonBinding = In_FindButtonMapping(inputContext, FREE_LOOK_CURSOR_SELECT_BINDING_NAME);
	pData->freeLookCtrls.exitFreeLookModeBinding = In_FindButtonMapping(inputContext, "exitFreeLookMode");

	In_ActivateButtonBinding(pData->freeLookCtrls.freeLookZoomInBinding, &pData->freeLookCtrls.freeLookInputsMask);
	In_ActivateButtonBinding(pData->freeLookCtrls.freeLookZoomOutBinding, &pData->freeLookCtrls.freeLookInputsMask);
	In_ActivateButtonBinding(pData->freeLookCtrls.freeLookZoomMoveXPosBinding, &pData->freeLookCtrls.freeLookInputsMask);
	In_ActivateButtonBinding(pData->freeLookCtrls.freeLookZoomMoveXNegBinding, &pData->freeLookCtrls.freeLookInputsMask);
	In_ActivateButtonBinding(pData->freeLookCtrls.freeLookZoomMoveYPosBinding, &pData->freeLookCtrls.freeLookInputsMask);
	In_ActivateButtonBinding(pData->freeLookCtrls.freeLookZoomMoveYNegBinding, &pData->freeLookCtrls.freeLookInputsMask);
	In_ActivateButtonBinding(pData->freeLookCtrls.freeLookSelectButtonBinding, &pData->freeLookCtrls.freeLookInputsMask);
	In_ActivateButtonBinding(pData->freeLookCtrls.exitFreeLookModeBinding, &pData->freeLookCtrls.freeLookInputsMask);
	In_ActivateAxisBinding(pData->freeLookCtrls.freeLookCursorXAxisBinding, &pData->freeLookCtrls.freeLookInputsMask);
	In_ActivateAxisBinding(pData->freeLookCtrls.freeLookCursorYAxisBinding, &pData->freeLookCtrls.freeLookInputsMask);
}
