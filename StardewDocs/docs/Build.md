# Build

The repo is a CMake project, and is built with GCC for linux and MSVC for windows.

## Dependencies

System supplied:

- libxml2/2.13.8 
- freetype/2.13.3
- lua/5.4.7
- glfw/3.4
- gtest/1.16.0 (optional for unit test project only, you can pass -DNoGTest=ON to cmake to remove this dependency and not build the test project)
- openal/1.22.2

These are the versions used in the windows build, gotten through the conan package manager.

Vendored:

- Box2D
- cJSON
- glad
- CGLM
- netcode (with vendored sodium library)
- stb image
- cwalk
- Zzfx-c (my project)

You can find these in the the repo at `engine/lib`, and for smaller libraries, `engine/src/vendor`. netcode is used by both the engine and matchmaking server (which uses no other part of the engine) so is in a separate directory and not the `engine/lib`. Vendored libraries may have one or two small tweaks, cwalk and netcode do. In the case of cwalk a stock version of the library would break the linux flatpak build. 

ZzFX-C is included as a git submodule, so remember to do:

`git submodules update --init`

before a local build.

## Build Time Dependencies

- Cmake
- GCC on linux, msvc on windows
- Python3
- Nushell (for "baked" item builds only)
- Bash on linux, cmd on windows

## Assets

As part of the build, asset files should be built. See [Asset Tools](./AssetTools.md) documentation for more details, as well as the compile_assets.sh script in the repo.

This isn't currently integrated into CMake and you've got to manually do it. On windows some shuffling of the built files is necessary for the game to then be playable. The compile_assets.bat script may or may not work properly as focus is primarily on the linux one, although I have tested a build from CI relatively recently and it worked fine.

## Build Flags

Pass these flags to the CMake build configure step, prefixed with `-D` (for example `cmake .. -DBAKE_ITEM_DEFS=ON`):

- `BAKE_ITEM_DEFS`
    - values: `ON`, `OFF`
- `STARDEW_PLATFORM`
    - values: `SDL2`, `GLFW3`

## Packaging

[This Repo](https://github.com/JimMarshall35/2DFarmingRPG-Flatpak) produces a flatpak build which I plan on being the main way of distributing a more complete version of the game for linux as it's building against a single set of dependencies and should work on most if not all linux distros. This build is in a separate repo that is supposed to be triggered weekly and generate a publicly downloadable `.flatpak` file. It uses the [flatpak-builder](https://docs.flatpak.org/en/latest/flatpak-builder.html) tool - This compiles the code with a different version of gcc than is used by the github CI and links against the free desktop SDK 2025 runtime. GLFW and lua 5.4 are both built from source by the flatpak-builder tool in this method.

There is also a script in the main repo that produces a debian package, but I don't plan on distributing the game this way.

The matchmaking server (which currently doesn't work) is built as a docker container - the best way for any kind of backend server.

I don't know the best way to package it for windows, I may build an installer with [Innosetup](https://jrsoftware.org/isinfo.php) as I have done in the past - but perhaps this is too "old school". The CI produces what I'm pretty sure should be a self contained folder with an .exe you can just play on windows (I've tested it on windows 10).

