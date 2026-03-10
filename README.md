# Readme
WORK IN PROGRESS

A 2D Game engine and farming RPG for windows and Linux.


## Try It Out
```bash
nix run github:JimMarshall35/2DFarmingRPG
```

# Engine Docs

engine/docs folder:

- [Asset Tools](Stardew/engine/docs/AssetTools.md)
- [Entities](Stardew/engine/docs/Entities.md)
- [Game](Stardew/engine/docs/Game.md)
- [UI](Stardew/engine/docs/UI.md)

doxygen docs:
https://jimmarshall35.github.io/2DFarmingRPG/

# Runtime Dependencies

System supplied:
(Should exist as system packages on linux, or gotten through conan package manager for windows build - version numbers are the versions conan fetches)
- libxml2/2.13.8 
- freetype/2.13.3
- lua/5.4.7
- glfw/3.4
- gtest/1.16.0 (optional for unit test project only, you can pass -DNoGTest=ON to cmake to remove this dependency and not build the test project)
- openal/1.22.2

Future work will be to make the linux build use conan for consistency, I don't mind having the vendored libraries

Vendored:
- Box2D
- cJSON
- glad
- CGLM
- netcode


# Build

To Build:
- Windows
  - Buildtime Dependencies
      - MSVC toolchain
      - Conan package manager
      - CMake
      - Python 3
  - Run GetDependenciesConan.bat
  - Run BuildRelease.bat
  - Run compile_assets.bat
  - Run BuildDebug.bat
- Linux (Ubuntu)
  - Buildtime Dependencies
    - GCC toolchain
    - CMake
    - Python 3
  - Run GetDependencies.sh
  - Run BuildDebug.sh
  - Run compile_assets.sh



# Nix

## Prerequisites

[Nix](https://nixos.org/download/) with flakes enabled. Add to `~/.config/nix/nix.conf`:

```
experimental-features = nix-flakes nix-command
```

## Dev shell

```bash
nix develop
```

Then use `play` to build and run the game.

## Build and run directly

```bash
nix run github:JimMarshall35/2DFarmingRPG
```

Or from a local clone:

```bash
nix run .
```
