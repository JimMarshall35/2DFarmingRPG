#ifndef FREELOOKCAMERAMODE_H
#define FREELOOKCAMERAMODE_H

#include "InputContext.h"
struct GameLayer2DData;
struct GameFrameworkLayer;

struct FreeLookCameraModeControls
{
    
	struct ButtonBinding freeLookZoomInBinding;
	struct ButtonBinding freeLookZoomOutBinding;

	struct ButtonBinding freeLookZoomMoveXPosBinding;
	struct ButtonBinding freeLookZoomMoveXNegBinding;

	struct ButtonBinding freeLookZoomMoveYPosBinding;
	struct ButtonBinding freeLookZoomMoveYNegBinding;

	struct ButtonBinding freeLookSelectButtonBinding;

	struct ButtonBinding exitFreeLookModeBinding;

	struct AxisBinding freeLookCursorXAxisBinding;
	struct AxisBinding freeLookCursorYAxisBinding;

	struct ActiveInputBindingsMask freeLookInputsMask;

};

extern float gDebugWSX;
extern float gDebugWSY;

void FreeLookMode2DInput(struct GameFrameworkLayer* pLayer, InputContext* context);
void BindFreeLookControls(InputContext* inputContext, struct GameLayer2DData* pData);

#endif