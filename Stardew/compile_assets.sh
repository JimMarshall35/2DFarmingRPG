# Compile assets for the game
#
#  (tiled jsons) + (source images) -> (.tilemap binary) + (.atlas binary)
#

# convert jsons from the Tiled editor to binary files containing tilemaps and entities + an atlas.xml file of the tiles used
python3 game/game_convert_tiled.py ./WfAssets/out -m ./WfAssets/Farm.json ./WfAssets/House.json ./WfAssets/RoadToTown.json 

# expand animation nodes
python3 ./engine/scripts/ExpandAnimations.py -o ./WfAssets/out/expanded_named_sprites.xml ./WfAssets/out/named_sprites.xml

# merge the list of named sprites into the ones used by the tilemap
python3 engine/scripts/MergeAtlases.py ./WfAssets/out/atlas.xml ./WfAssets/out/expanded_named_sprites.xml > ./WfAssets/out/atlascombined.xml

# compile the atlascombined.xml into a binary atlas file
./build/atlastool/AtlasTool ./WfAssets/out/atlascombined.xml -o ./WfAssets/out/main.atlas -bmp Atlas.bmp

# compile another atlas file containing sprites and fonts for the games UI 
./build/atlastool/AtlasTool ./WfAssets/ui_atlas.xml -o ./WfAssets/ui_atlas.atlas -bmp UIAtlas.bmp

# make a dev save file (temporary measure)
./build/game/WarFarmer --outPersistantFile ./WfAssets/Saves/Dev/Persistant.game

# copy tilemap files into the dev save folder
cp ./WfAssets/out/Farm.tilemap ./WfAssets/Saves/Dev
cp ./WfAssets/out/House.tilemap ./WfAssets/Saves/Dev
cp ./WfAssets/out/RoadToTown.tilemap ./WfAssets/Saves/Dev

#rm -f ./WfAssets/out/atlascombined.xml
#rm -f ./WfAssets/out/expanded_named_sprites.xml

