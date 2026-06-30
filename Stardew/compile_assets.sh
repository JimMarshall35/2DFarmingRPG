#!/usr/bin/env bash

# Compile assets for the game
#
#  (tiled jsons) + (source images) -> (.tilemap binary) + (.atlas binary)
#
map_files=(
    ./WfAssets/Farm.json 
    ./WfAssets/House.json 
    ./WfAssets/RoadToTown.json
    ./WfAssets/Town.json
)

map_filenames=(
    Farm
    House
    RoadToTown
    Town
)

# convert jsons from the Tiled editor to binary files containing tilemaps and entities + an atlas.xml file of the tiles used
python3 -m game.game_convert_tiled ./WfAssets/out -m "${map_files[@]}" 

# expand templates
python3 ./engine/engine/scripts/ExpandAtlasTemplate.py ./WfAssets/out/named_sprites.xml -o ./WfAssets/out/named_sprites_expanded_templates.xml

# expand animation nodes
python3 ./engine/engine/scripts/ExpandAnimations.py -o ./WfAssets/out/expanded_named_sprites.xml ./WfAssets/out/named_sprites_expanded_templates.xml

# merge the list of named sprites into the ones used by the tilemap
python3 ./engine/engine/scripts/MergeAtlases.py ./WfAssets/out/atlas.xml ./WfAssets/out/expanded_named_sprites.xml > ./WfAssets/out/atlascombined.xml

# compile the atlascombined.xml into a binary atlas file
./build/engine/atlastool/AtlasTool ./WfAssets/out/atlascombined.xml -o ./WfAssets/out/main.atlas -bmp Atlas.bmp -iw 1024 -ih 1024

# compile another atlas file containing sprites and fonts for the games UI 
./build/engine/atlastool/AtlasTool ./WfAssets/ui_atlas.xml -o ./WfAssets/ui_atlas.atlas -bmp UIAtlas.bmp -iw 1024 -ih 1024

# make a dev save file (temporary measure)
./build/game/WarFarmer --outPersistantFile ./WfAssets/Saves/Dev/Persistant.game

# copy tilemap files into the dev save folder

for item in "${map_filenames[@]}"; do
  cp "./WfAssets/out/$item.tilemap" -f ./WfAssets/Saves/Dev
done

#rm -f ./WfAssets/out/atlascombined.xml
#rm -f ./WfAssets/out/expanded_named_sprites.xml

