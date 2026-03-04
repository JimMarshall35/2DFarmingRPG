# Game Framework

The engine is based on a simple "Game Framework" concept. This provides a stack of "layers" which may be pushed and popped. A layer is basically a set of callbacks that you can implement to implement a game such as Update, Draw, Input, etc.

The idea behind the layered concept is that layers can mask the ones below and either mask draw, update, input, or all 3. This means that a pause game framework layer can be pushed on top of the game game framework layer, masking its update and input callbacks but not the draw: the game is still rendered below the pause menu but the game is paused.

Another example would be: you travel to a new area in the game, and push that layer on top of the old one, masking it. When you leave the area back to the old one, the game framework layer is just popped off

This is a pretty simple and versatile way to have:
    - different UI screens
    - pause menus
    - different game areas concurrently loaded
    - game HUDs

The engine implements two different game framework layers:
    - UI
        - a declarative UI layer based on xml and lua code (see UI.md)
    - Game2D
        - a framework for making 2D games
            - an entity system
            - a tilemap system
            - functions to serialize and deserialize
            - drawing and culling routines 
            - 2D physics (using box2d library)
            - structure of the main loop for this type of game
            - networking

The game implemented in Stardew/src extends the Game2D layer, the idea is that a radically different type of game (such as a 3d game) could be created by implementing a Game3D layer (for example), and much of the rest of the engine would still be useful including the core library, core networking, input and the UI layer (and when i implement it, audio). New rendering functions would have to be implemented of course.

