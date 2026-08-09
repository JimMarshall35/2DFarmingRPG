#!/usr/bin/env nu

#
#
#   This is the master script, similar to how some projects use a makefile, except this doesn't have
#   a makefiles incremental task running behavior. It allows reuse of the same scripts for CI and local development
#   and between windows and linux (and mac if i add it in the future).
#
#

use std/dirs

def compile_assets [
    atlas_tool_executable: string, #./build/engine/atlastool/AtlasTool
    game_executable: string,       #./build/game/WarFarmer
    atlas_tool_args: list
] {
    let map_files = [
        ./WfAssets/Farm.json
        ./WfAssets/House.json
        ./WfAssets/RoadToTown.json
        ./WfAssets/Town.json
    ]

    let map_filenames = [
        Farm
        House
        RoadToTown
        Town
    ]
    if (not ($atlas_tool_executable | path exists )) {
        error make {msg: $"atlas_tool_executable doesn't exist ($atlas_tool_executable)"}
    } 
    if (not ($game_executable | path exists )) {
        error make {msg: $"game_executable doesn't exist ($game_executable)"}
    } 
    print $"atlas_tool_executable: ($atlas_tool_executable)"
    print $"game_executable: ($game_executable)"

    # convert jsons from the Tiled editor to binary files containing tilemaps and entities + an atlas.xml file of the tiles used
    python3 -m game.game_convert_tiled ./WfAssets/out -m ...$map_files

    print "EXPANDING TEMPLATES\n"

    python3 ./engine/engine/scripts/ExpandAtlasTemplate.py ./WfAssets/out/named_sprites.xml -o ./WfAssets/out/named_sprites_expanded_templates.xml

    print "EXPANDING ANIMATION NODES\n"

    python3 ./engine/engine/scripts/ExpandAnimations.py -o ./WfAssets/out/expanded_named_sprites.xml ./WfAssets/out/named_sprites_expanded_templates.xml


    print "MERGING HAND MADE AND TILEMAP ATLAS XMLS\n"

    python3 ./engine/engine/scripts/MergeAtlases.py ./WfAssets/out/atlas.xml ./WfAssets/out/expanded_named_sprites.xml | save -f ./WfAssets/out/atlascombined.xml

    print "BUILDING MAIN .atlas FILE\n"

    let main_atlas_args = [
        "./WfAssets/out/atlascombined.xml",
        "-o",
        "./WfAssets/out/main.atlas"
    ]
    let main_atlas_args = $main_atlas_args ++ $atlas_tool_args

    print $main_atlas_args

    ^$atlas_tool_executable ...$main_atlas_args
    print $"exit code: ($env.LAST_EXIT_CODE)"
    if $env.LAST_EXIT_CODE != 0 {
        error make {msg: "AtlasTool failed on main atlas"}
    }

    print "BUILDING UI .atlas FILE\n"

    let ui_atlas_args = [
        "./WfAssets/out/ui_atlas.xml",
        "-o",
        "./WfAssets/out/ui_atlas.atlas"
    ]
    let ui_atlas_args = $ui_atlas_args ++ $atlas_tool_args

    print $ui_atlas_args 

    # compile another atlas file containing sprites and fonts for the games UI
    ^$atlas_tool_executable ...$ui_atlas_args

    print "MAKING DEV SAVE\n"

    ^$game_executable --outPersistantFile ./WfAssets/Saves/Dev/Persistant.game

    print "COPYING TILEMAPS TO DEV SAVE\n"

    for item in $map_filenames {
        cp -f $"./WfAssets/out/($item).tilemap" ./WfAssets/Saves/Dev
    }

    print "CLEANING UP\n"
    rm -f ./WfAssets/out/atlascombined.xml
    rm -f ./WfAssets/out/expanded_named_sprites.xml
    rm -f ./WfAssets/out/named_sprites_expanded_templates.xml
    rm -f ./WfAssets/out/atlas.xml
}

def copy_built_assets_to_dir [
    destination: string
] {
    let src = "./WfAssets" | path expand
    let dst = $destination #"./build/install_dir/Warfarmer/WfAssets"
    let exts = [txt tilemap atlas xml lua game json]

    let pattern = $"./WfAssets/**/*.{($exts | str join ',')}"

    for file in (glob $pattern) {
        print $"file ($file) src ($src)"
        let rel = ($file | str replace $"($src)/" "")
        print $"rel ($rel)"
        let dest_path = $"($dst)/($rel)"
        print $"copying file ($file) to ($dest_path)"
        mkdir ($dest_path | path dirname)
        cp -f $file $dest_path
    }
}

def "main compile_assets_linux" [] {
    ( compile_assets 
    "./build/engine/atlastool/AtlasTool" 
    "./build/game/WarFarmer"
    ["-iw", 1024, "-ih", 1024] )
}

def "main compile_assets_windows" [] {
    ( compile_assets 
    "./build/install_dir/Warfarmer/AtlasTool.exe" 
    "./build/install_dir/Warfarmer/WarFarmer.exe"
    [])
    copy_built_assets_to_dir ./build/install_dir/Warfarmer/WfAssets
}

def "main get_dependencies_conan" [build_type: string] {
    mkdir build
    conan profile detect --force
    dirs add ./build
    conan install .. -s $"build_type=($build_type)" -o "*:shared=False" --deployer=full_deploy --deployer-folder=./ --build=missing
    dirs drop
}

def "main get_dependencies_apt" [] {
    apt-get update
    # for some reason the appimage tool needs libfuse2t64
    apt-get install -y libxml2 liblua5.3-dev libxml2-dev libbox2d-dev libglfw3-dev libfreetype-dev libgtest-dev libopenal-dev libsdl2-dev libfuse2t64
}

def "main strip_unnecessary_conan_deps" [
    platform: string,
    sdl_package: string,
    glfw_package: string
] {
    if $platform == "SDL2" {
        open conanfile.txt 
        | str replace -a $glfw_package '' 
        | save -f conanfile.txt
    } else if $platform == "GLFW3" {
        open conanfile.txt 
        | str replace -a $sdl_package '' 
        | save -f conanfile.txt
    } else {
        error make {msg: $"invalid platform option ($platform)"}
    }
    cat conanfile.txt
}

def "main stamp_version" [
    baked: bool,
    platform: string,
    type: string
] {
    if $platform not-in ["SDL2", "GLFW3"] {
        error make {msg: $"invalid platform: ($platform)"}
    }
    if $type not-in ["Release", "Debug"] {
        error make {msg: $"invalid type: ($type)"}
    }

    dirs add ./game
    if $baked {
        python3 make_release_version_header.py --platform $platform --build_type $type --baked
    } else {
        python3 make_release_version_header.py --platform $platform --build_type $type
    }
    dirs drop
}

def build_linux [
    build_type: string,
    bake_item_defs: bool,
    platform: string,
    gl_api: string
] {
    mkdir build
    dirs add ./build
    let baked = if $bake_item_defs {
        "ON"
    } else {
        "OFF"
    }
    let args = [
        $"-DCMAKE_BUILD_TYPE=($build_type)",
        $"-DBAKE_ITEM_DEFS=($baked)",
        $"-DSTARDEW_PLATFORM=($platform)",
        $"-DSTARDEW_GL_API_TYPE=($gl_api)"
    ]
    cmake .. ...$args
    make

    dirs drop
    copy_built_assets_to_dir "./build/game"
}

def build_windows [
    build_type: string,
    bake_item_defs: bool,
    platform: string,
    gl_api: string
] {
    mkdir build
    dirs add ./build
    mkdir install_dir

    let baked = if $bake_item_defs {
        "ON"
    } else {
        "OFF"
    }
    #cmake .. -G "Visual Studio 18 2026" -DCMAKE_TOOLCHAIN_FILE=generators\conan_toolchain.cmake  -DCMAKE_POLICY_DEFAULT_CMP0091=NEW -DCMAKE_WINDOWS_EXPORT_ALL_SYMBOLS=TRUE -DBAKE_ITEM_DEFS=%2 -DSTARDEW_PLATFORM=%3 -DSTARDEW_GL_API_TYPE=%4

    let args = [
        #"-G",
        #"\"Visual Studio 18 2026\"",
        "-DCMAKE_TOOLCHAIN_FILE=generators\\conan_toolchain.cmake",
        "-DCMAKE_POLICY_DEFAULT_CMP0091=NEW",
        "-DCMAKE_WINDOWS_EXPORT_ALL_SYMBOLS=TRUE"
        $"-DBAKE_ITEM_DEFS=($baked)",
        $"-DSTARDEW_PLATFORM=($platform)",
        $"-DSTARDEW_GL_API_TYPE=($gl_api)"
    ]
    cmake .. ...$args
    cmake --build . --config $build_type
    cmake --install . --prefix ./install_dir --config $build_type

    dirs drop
}

def "main build_linux" [
    build_type: string,
    bake_item_defs: bool,
    platform: string,
    gl_api: string
] {
    build_linux $build_type $bake_item_defs $platform $gl_api
}

def "main build_linux_dev" [
] {
    (build_linux 
        "Debug" 
        false
        "GLFW3"
        "OPENGLES")
}

def "main build_windows" [
    build_type: string,
    bake_item_defs: bool,
    platform: string,
    gl_api: string
] {
    (build_windows 
    $build_type 
    $bake_item_defs
    $platform 
    $gl_api)
}

def "main build_debian" [

] {
    ./debian/BuildDebianPkg.sh
}

def "main run_tests_linux" [

] {
    ./build/engine/enginetest/StardewEngineTest
}

def "main build_appimage" [

] {
    ./appimage/BuildAppImage.sh
}

def "main launch_game_client_linux" [

] {
    ./build/game/WarFarmer -r c -s 127.0.0.1:666 -l i
}

def "main launch_game_server_linux" [

] {
    ./build/game/WarFarmer -r s -s 127.0.0.1:666 -l i 
}


def "main run_tests_windows" [

] {
    ./build/install_dir/Tests/StardewEngineTest.exe
}

def main [
] {
    
}   
