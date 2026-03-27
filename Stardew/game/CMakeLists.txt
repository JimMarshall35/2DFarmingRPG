add_executable( WarFarmer
    src/main.c
    src/WfEntities.c
    src/WfInit.c
    src/WfGame.c
    src/WfGameLayerData.c
    src/WfSprites.c
    src/WfWorld.c
    src/WfGameLayer.c
    src/WfScriptFunctions.c
    src/WfNetwork.c
    src/entities/WfEntityMessages.c
    src/entities/WfDebrisField.c
    src/entities/WfExit.c
    src/entities/WfLog.c
    src/entities/WfPlayer.c
    src/entities/WfPlayerStart.c
    src/entities/WfRock.c
    src/entities/WfTree.c
    src/entities/WfWoodedArea.c
    src/entities/WfItemPickup.c
    src/nonEntityGameData/WfPersistantGameData.c
    src/nonEntityGameData/WfItem.c
    src/items/WfItemHelpers.c
    src/items/WfBasicAxe.c
    src/items/WfBasicBow.c
    src/items/WfBasicFishingRod.c
    src/items/WfBasicHoe.c
    src/items/WfBasicPickaxe.c
    src/items/WfBasicScythe.c
    src/items/WfBasicSword.c
    src/items/WfWoodItem.c
    src/items/WfDryGroundLUT.c
    src/ui/WfUI.c
    src/WfVersion.c
)

target_include_directories(WarFarmer PRIVATE include)