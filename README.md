# Readme
WORK IN PROGRESS

A 2D Game engine and farming RPG for windows and Linux.

# Docs

- [Asset Tools](Stardew/engine/docs/AssetTools.md)
- [Entities](Stardew/engine/docs/Entities.md)
- [Game](Stardew/engine/docs/Game.md)
- [UI](Stardew/engine/docs/UI.md)

# Build

To Build:
- Windows
  - Buildtime Dependencies
      - MSVC toolchain
      - Conan package manager
      - CMake
  - Run GetDependenciesConan.bat
  - Run BuildRelease.bat
  - Run compile_assets.bat (this likely won't as it hasn't been maintained - copy the linux .sh one)
  - Run BuildDebug.bat
- Linux (Ubuntu)
  - Buildtime Dependencies
    - GCC toolchain
    - CMake
  - Run GetDependencies.sh
  - Run BuildDebug.sh
  - Run compile_assets.sh
