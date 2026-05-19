#ifndef  HANDLE_DEFS_H
#define HANDLE_DEFS_H

#include "IntTypes.h"

typedef i32 HGeneric;

typedef HGeneric HWidget;
#define NULL_HWIDGET -1

typedef HGeneric HImage;
#define NULL_HIMAGE -1

typedef HGeneric hAtlas;
#define NULL_ATLAS -1

typedef HGeneric hSprite;
#define NULL_HSPRITE -1

typedef HGeneric hTexture;

typedef HGeneric HWorldspaceVertexBuffer;

typedef HGeneric HUIVertexBuffer;

typedef HGeneric HWorldspaceLineVertexBuffer;

typedef HGeneric H2DWorldspaceVertexBuffer;

typedef HGeneric HFont;

typedef HGeneric HMouseAxisBinding;

typedef HGeneric HMouseButtonBinding;

typedef HGeneric HKeyboardButtonBinding;

typedef HGeneric HTimer;

typedef HGeneric HEntity2D;

typedef HGeneric HPhysicsWorld;

typedef HGeneric H2DBody;

typedef HGeneric HEntity2DQuadtreeNode;

typedef HGeneric HEntity2DQuadtreeEntityRef;

typedef HGeneric HDynamicEntityListItem;

#define NULL_HANDLE -1


#endif