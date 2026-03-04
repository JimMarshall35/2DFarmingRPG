# Game

## Engine Design Principals

The engine is designed to provide a main loop for the game and the have the game overrides callbacks. The distinction between engine and game level code is clear but also new bits of "engine" level code can just as easily be added in the game executable if desired and then potentially moved to the engine later if deemed that they can be reused. First and foremost the game engine desires to provide solid and easy to use components that **virtually all** games will use (such as UI and sound, networking, input). Less focus has been placed on rendering as different games may want to take drastically different approaches. The game engine also provides "game layers" (see game framework documentation) to ease the creation of specific types of games (right now Game2D, a networked 2d game with tilemaps). If you want to make a radically different type of game, you'll implement a new game layer for your specific game type. This will include:

- the entity system
- the update / physics code
- the drawing code

but will not include:
- input
- scene management
- UI
- sound (not yet implemented)
- low level networking

The foundational core engine layer and game framework should provide a useful context to the implementation of new game layers, which should be in turn extensible by **games** of a particular type.

If and when new game framework layer types get implemented (for a 3d game for example) my hope is that code specific to that layer would probably find its way upstream into the shared rendering code DrawContext.c and other lower level libraries.

DrawContext.c should implement **low level** rendering operations.

## Creating a game

To create a game, create a new executable that links to the engine:

```cmake
cmake_minimum_required(VERSION 3.25)
project(MyGame)
add_subdirectory(game) # defines the target Game

target_link_libraries(Game PUBLIC StardewEngine)
```

In the main function of the game, call EngineStart, passing in a GameInit function that pushes a game framework layer to start your game.
Your game can define its own game framework layer or use a built in one.


Here's an example of starting a game using the builtin Game2DLayer:

```c
/* */

#include "main.h"
#include "GameFramework.h"
#include <string.h>
#include "Game2DLayer.h"
#include "XMLUIGameLayer.h"
#include "DynArray.h"
#include "Entities.h"
#include "EntityQuadTree.h"
#include "WfEntities.h"
#include "Physics2D.h"
#include "WfInit.h"
#include "Random.h"

void GameInit(InputContext* pIC, DrawContext* pDC)
{
    // init any engine systems - just an example
    unsigned int seed = Ra_SeedFromTime();
    printf("seed: %u\n", seed);
    WfInit();
    Ph_Init();
    InitEntity2DQuadtreeSystem();
    Et2D_Init(&WfRegisterEntityTypes);

    struct GameFrameworkLayer testLayer;
    memset(&testLayer, 0, sizeof(struct GameFrameworkLayer));
    struct Game2DLayerOptions options;
    memset(&options, 0, sizeof(struct Game2DLayerOptions));
    options.atlasFilePath = "./WfAssets/out/main.atlas";
    options.tilemapFilePath = "./WfAssets/out/Farm.tilemap";
    Game2DLayer_Get(&testLayer, &options, pDC);
    testLayer.flags |= (EnableOnPop | EnableOnPush | EnableUpdateFn | EnableDrawFn | EnableInputFn);
    GF_PushGameFrameworkLayer(&testLayer);
}

int main(int argc, char** argv)
{

    EngineStart(argc, argv, &GameInit);
}
```

If you use the built in Game2DLayer your game can register entity types with asset tooling and the entity system.

For more information on this see Entities.md and Assets.md

