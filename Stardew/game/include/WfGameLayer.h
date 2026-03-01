#ifndef WFGAMELAYER_H
#define WFGAMELAYER_H

struct DrawContext;
typedef struct DrawContext DrawContext;

void WfPushGameLayer(DrawContext* pDC, const char* lvlFilePath);
void WfPublishInventoryChangedEvent();

#endif
