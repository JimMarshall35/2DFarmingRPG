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
          ];

          # Make pkg-config and headers discoverable
          nativeBuildInputs = with pkgs; [
            pkg-config
          ];

          shellHook = ''
            echo ""
            echo "🌾 2D Farming RPG dev environment"
            echo ""
            echo "  Build commands:"
            echo "    bash BuildDebug.sh     - debug build"
            echo "    bash GetDependencies.sh is NOT needed — nix provides all deps"
            echo ""
          '';
        };
      }
    );
}
