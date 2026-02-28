#include "WfUI.h"
#include "GameFramework.h"
#include "XMLUIGameLayer.h"
#include "DrawContext.h"
#include "Log.h"
#include "main.h"
#include "cwalk.h"
#include <string.h>

void WfPushHUD(DrawContext* pDC)
{
    struct GameFrameworkLayer testLayer;
    memset(&testLayer, 0, sizeof(struct GameFrameworkLayer));
    struct XMLUIGameLayerOptions options;
    char buf[256];
    cwk_path_join(gCmdArgs.assetsDir, "GameHUD.xml", buf, 256);
    options.xmlPath = buf;
    options.pDc = pDC;
    testLayer.flags |= (EnableOnPush | EnableOnPop);
    Log_Verbose("making xml ui layer");
    XMLUIGameLayer_Get(&testLayer, &options);
    Log_Verbose("done");
    Log_Verbose("pushing framework layer");
    GF_PushGameFrameworkLayer(&testLayer);
}

void WfPushSettings(DrawContext* pDC)
{
    struct GameFrameworkLayer testLayer;
    memset(&testLayer, 0, sizeof(struct GameFrameworkLayer));
    struct XMLUIGameLayerOptions options;
    char buf[256];
    cwk_path_join(gCmdArgs.assetsDir, "Settings.xml", buf, 256);
    options.xmlPath = buf;
    options.pDc = pDC;
    Log_Verbose("making xml ui layer");
    XMLUIGameLayer_Get(&testLayer, &options);
    testLayer.flags |= ( MasksUpdate | MasksInput );

    Log_Verbose("done");
    Log_Verbose("pushing framework layer");
    GF_PushGameFrameworkLayer(&testLayer);
}
