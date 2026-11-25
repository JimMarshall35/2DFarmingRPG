
{
  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";

  outputs = { self, nixpkgs }:
    let
      system = "x86_64-linux";
      pkgs = nixpkgs.legacyPackages.${system};
      deps = with pkgs; [ lua5_3 libxml2 box2d freetype gtest openal SDL2 glfw ];

      gameWrapper = pkgs.writeShellScriptBin "2DFarmingRPGNixosWrapper" ''
        ASSETSDIR="${self.packages.${system}.default}/share/WfAssets"
        SAVESDIR="~/Stardew/Saves"
        CONFIGDIR="~/Stardew/Config"

        mkdir -p $SAVESDIR
        mkdir -p $CONFIGDIR
        exec ${self.packages.${system}.default}/bin/WarFarmer \
          --assetsDir $ASSETSDIR \
          --configDir $CONFIGDIR \
          --savesDir $SAVESDIR \
          "$@"
      '';

    in {
      packages.${system}.default = pkgs.stdenv.mkDerivation {
        pname = "2DFarmingRPG";
        version = "0.1";
        src = self;
        sourceRoot = "source/Stardew";

        nativeBuildInputs = [
          pkgs.cmake
          pkgs.pkg-config
          pkgs.nushell
          pkgs.patchelf
          (pkgs.python3.withPackages (ps: with ps; [
            jinja2
          ]))
        ];

        buildInputs = deps;

        dontUseCmakeConfigure = true;

        configurePhase = ''
          runHook preConfigure
          runHook postConfigure
        '';

        buildPhase = ''
          runHook preBuild
          nu ./Makefile.nu build_linux_release
          nu ./Makefile.nu compile_assets_linux
          runHook postBuild
        '';

        doCheck = false;

        installPhase = ''
          runHook preInstall
          cmake --install build --prefix $out
          runHook postInstall
        '';

        postFixup = ''
          patchelf --add-rpath '$ORIGIN/../lib' "$out/bin/WarFarmer"
        '';
      };

      devShells.${system}.default = pkgs.mkShell {
        nativeBuildInputs = [ 
          pkgs.cmake 
          pkgs.pkg-config 
          pkgs.nushell 
          (pkgs.python3.withPackages (ps: with ps; [
            jinja2
          ]))
        ];
        buildInputs = deps;
        shellHook = ''
          export PS1="\[\e[1;32m\]🌽 \u@\h:\w\$ \[\e[0m\]"
        '';
      };

      apps.${system}.default = {
        type = "app";
        program = "${gameWrapper}/bin/2DFarmingRPGNixosWrapper";
      };
    };
}
