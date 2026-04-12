option(BAKE_ITEM_DEFS "Use baked in item defintions" OFF)


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
    src/entities/WfPlayer.c
    src/entities/WfPlayerStart.c
    src/entities/WfDebris.c
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


target_compile_definitions(WarFarmer PRIVATE
    BAKE_ITEM_DEFS=$<BOOL:${BAKE_ITEM_DEFS}>
)

target_include_directories(WarFarmer PRIVATE include)

if(BAKE_ITEM_DEFS)
    target_sources(WarFarmer PRIVATE src/BakedItems.c)
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_LIST_DIR}/src/BakedItems.c ${CMAKE_CURRENT_LIST_DIR}/include/BakedItems.h
        COMMAND ${CMAKE_CURRENT_LIST_DIR}/ma_baker.nu ${CMAKE_CURRENT_LIST_DIR}/../WfAssets/items_data.xml --out_c_folder ${CMAKE_CURRENT_LIST_DIR}/src --out_h_folder ${CMAKE_CURRENT_LIST_DIR}/include
    )
endif()

if(WIN32)
else()
    target_link_options(WarFarmer PRIVATE -rdynamic)
endif()