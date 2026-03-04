# Rendering

Currently OpenGL ES is used for rendering. Originally I had used desktop openGL, and traces of this code remain, but this should be removed. In future I'd like to support a vulkan backend as well as opengl ES.

The concept for rendering the Game2D and UI layers is simple, a buffer of verts is populated each frame and is then drawn.

- each frame there are two glBufferSubData calls (for game and then UI) and two draw calls (for game and UI), and that's more or less it for openGL calls. This should be simple to port to other rendering apis.
    - this is possible because a per-gamelayer texture atlas is used
        - in future I'd like to expand this to be **up to 16** texture atlases per game framework layer, ie make use of all texture slots and have 3d UVs
        - max opengl texture size is something like 3000x3000
- the game uses indexed vertices, the UI doesn't
- ui verts have colour attributes (for text colour)
- the game only draws tiles and entities that are in the viewport 
    - static entities are kept in a quad tree
    - dynamic ones searched linearly
    - start and finish index of tiles to draw found simply from viewport topleft/bottomright



