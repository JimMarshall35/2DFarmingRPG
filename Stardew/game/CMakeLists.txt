add_executable( WarFarmer
    src/main.c
    src/cwalk.c
    src/WfEntities.c
    src/WfInit.c
    src/WfGame.c
    src/WfGameLayerData.c
    src/WfSprites.c
    src/WfWorld.c
    src/WfGameLayer.c
    src/entities/WfDebrisField.c
    src/entities/WfExit.c
    src/entities/WfLog.c
    src/entities/WfPlayer.c
    src/entities/WfPlayerStart.c
    src/entities/WfRock.c
    src/entities/WfTree.c
    src/entities/WfWoodedArea.c
    src/nonEntityGameData/WfItem.c
    src/items/WfBasicAxe.c
    src/items/WfBasicBow.c
    src/items/WfBasicFishingRod.c
    src/items/WfBasicHoe.c
    src/items/WfBasicPickaxe.c
    src/items/WfBasicScythe.c
    src/items/WfBasicSword.c
    src/ui/WfHUD.c
)

target_include_directories(WarFarmer PRIVATE include)