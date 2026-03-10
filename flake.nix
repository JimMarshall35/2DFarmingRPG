{
  description = "2D Game Engine and Farming RPG - development environment";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs =
    {
      self,
      nixpkgs,
      flake-utils,
    }:
    flake-utils.lib.eachDefaultSystem (
      system:
      let
        pkgs = nixpkgs.legacyPackages.${system};

        play = pkgs.writeShellScriptBin "play" ''
          cd "$(git rev-parse --show-toplevel)/Stardew"
          bash BuildDebug.sh && \
          bash compile_assets.sh && \
          ./build/game/WarFarmer
        '';
      in
      {
        devShells.default = pkgs.mkShell {
          name = "farming-rpg-dev";

          packages = with pkgs; [
            # Build toolchain
            gcc
            cmake
            gnumake
            python3

            # Runtime / system dependencies (mirrors GetDependencies.sh)
            libxml2
            libxml2.dev
            lua5_4
            glfw
            freetype
            gtest
            openal

            # Useful dev tools
            gdb
            valgrind
            doxygen

            # Dev scripts
            play
          ];

          nativeBuildInputs = with pkgs; [
            pkg-config
          ];

          shellHook = ''
            echo ""
            echo "🌾 2D Farming RPG dev environment"
            echo ""
            echo "  Commands:"
            echo "    play   - build, copy assets, and run the game"
            echo ""
          '';
        };
      }
    );
}
