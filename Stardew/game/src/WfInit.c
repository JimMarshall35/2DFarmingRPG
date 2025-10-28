#include "WfInit.h"
#include "WfWoodedArea.h"
#include "WfPlayerStart.h"
#include "WfPlayer.h"
#include "WfWorld.h"
#include "WfExit.h"
#include "WfItem.h"

void WfInit()
{
    WfWoodedAreaInit();
    WfInitPlayerStart();
    WfInitPlayer();
    WfWorldInit();
    WfInitExit();
    WfInitItems();
}

