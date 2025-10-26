# Compile assets for the game
#
#  (tiled jsons) + (source images) -> (.tilemap binary) + (.atlas binary)
#

# convert jsons from the Tiled editor to binary files containing tilemaps and entities + an atlas.xml file of the tiles used
python3 game/game_convert_tiled.py ./Assets/out -m ./Assets/Farm.json ./Assets/House.json ./Assets/RoadToTown.json 

# expand animation nodes
python3 ./engine/scripts/ExpandAnimations.py -o ./Assets/out/expanded_named_sprites.xml ./Assets/out/named_sprites.xml

# merge the list of named sprites into the ones used by the tilemap
python3 engine/scripts/MergeAtlases.py ./Assets/out/atlas.xml ./Assets/out/expanded_named_sprites.xml > ./Assets/out/atlascombined.xml

# compile the atlascombined.xml into a binary atlas file
./build/atlastool/AtlasTool ./Assets/out/atlascombined.xml -o ./Assets/out/main.atlas -bmp Atlas.bmp

# compile another atlas file containing sprites and fonts for the games UI 
./build/atlastool/AtlasTool ./Assets/ui_atlas.xml -o ./Assets/ui_atlas.atlas -bmp UIAtlas.bmp

# copy tilemap files into the dev save folder
cp ./Assets/out/Farm.tilemap ./Assets/Saves/Dev
cp ./Assets/out/House.tilemap ./Assets/Saves/Dev
cp ./Assets/out/RoadToTown.tilemap ./Assets/Saves/Dev