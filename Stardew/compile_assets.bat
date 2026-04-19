
python -m game.game_convert_tiled ./WfAssets/out -m ./WfAssets/Farm.json ./WfAssets/House.json ./WfAssets/RoadToTown.json ./WfAssets/Town.json -a engine/scripts/AtlasTool.exe

python ./engine/scripts/ExpandAnimations.py -o ./WfAssets/out/expanded_named_sprites.xml ./WfAssets/out/named_sprites.xml

python engine/scripts/MergeAtlases.py ./WfAssets/out/atlas.xml ./WfAssets/out/expanded_named_sprites.xml > ./WfAssets/out/atlascombined.xml

"./build/install_dir/Warfarmer/AtlasTool.exe" ./WfAssets/out/atlascombined.xml -o ./WfAssets/out/main.atlas

"./build/install_dir/Warfarmer/AtlasTool.exe" ./WfAssets/ui_atlas.xml -o ./WfAssets/ui_atlas.atlas

"./build/install_dir/Warfarmer/WarFarmer.exe" --outPersistantFile ./WfAssets/Saves/Dev/Persistant.game

robocopy "./WfAssets/" ./"build/install_dir/Warfarmer/WfAssets/" /E /XO *.txt *.tilemap *.atlas *.xml *.lua *.game *.json
del "./WfAssets/out/atlascombined.xml" 
del "./WfAssets/out/expanded_named_sprites.xml" 

pause