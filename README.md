# Readme
WORK IN PROGRESS

A 2D Game engine and farming RPG for windows and Linux.

# Engine Docs

engine/docs folder:

- [Asset Tools](Stardew/engine/docs/AssetTools.md)
- [Entities](Stardew/engine/docs/Entities.md)
- [Game](Stardew/engine/docs/Game.md)
- [UI](Stardew/engine/docs/UI.md)

doxygen docs:
https://jimmarshall35.github.io/2DFarmingRPG/

# Runtime Dependencies

See Makefile.nu or the conanfile.txt (note that either SDL2 OR GLFW3 is required, not both).

# Build

Some projects use a makefile as a collection of top level scripts to build, install and test the applicatione etc.
Being able to develop and compile on both linux and natively on windows has always been a goal of this project, and so a makefile is perhaps not ideal. What I want is a common shell scripting language for both windows and linux, and the one I've chosen is nushell.

This is a really nice shell and basically a functional programming language, and so it's ideal for doing both shell stuff and stuff that I might have previously written a python script for.

It does mean that you have to install this as a build time dependency however.

Build time dependencies Windows:
- msvc / visual studio
- nushell
- python 3
- CMake
- Conan package manager

Build time dependencies on linux
- gcc toolchain
- nushell
- python3
- CMake

To build on ubuntu run these commands from the repository's root:

```bash
cd Stardew
./Makefile.nu get_dependencies_apt
./Makefile.nu build_linux_dev                 # or just "build_linux" if you want to specify different build options
./Makefile.nu compile_assets_linux
# you now have a build in Stardew/build/game
```

To build on windows run this:

```
cd Stardew
nu ./Makefile.nu get_dependencies_conan "Debug"               # chose "Debug" or "Release", you might want to run both
nu ./Makefile.nu build_windows "Debug" false "GLFW3" "OPENGL" # select other options if you want
nu ./Makefile.nu compile_assets_windows
# you should now have a build in Stardew/build/game
```

# Developing with vscode

Copy the files in JimsVSCodeFiles into a folder called .vscode in the projects root or run JimsVSCodeFiles/Install.sh from the JimsVSCodeFiles folder to be able to debug the game with gdb inside vscode on linux (run the "Launch Game" from the gui)
 
# Authoring new content

To create levels and edit existing ones you need the GUI level editor program "Tiled". Open the file "Stardew/WfAssets/Engine.tiled-project". See docs for more details on how to add new content.
