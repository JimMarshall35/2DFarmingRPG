#ifndef WFGAMELAYER_H
#define WFGAMELAYER_H

struct DrawContext;
typedef struct DrawContext DrawContext;
struct InputContext;
typedef struct InputContext InputContext;

void WfPushGameLayer(DrawContext* pDC, const char* lvlFilePath);
void WfPublishInventoryChangedEvent();
void WfGameLayerOnPush(struct GameFrameworkLayer* pLayer, DrawContext* drawContext, InputContext* inputContext);
void WfGameLayerOnPop(struct GameFrameworkLayer* pLayer, DrawContext* drawContext, InputContext* inputContext);
void WfPreLoadLevel(struct GameLayer2DData* pEngineLayer);

#endif
