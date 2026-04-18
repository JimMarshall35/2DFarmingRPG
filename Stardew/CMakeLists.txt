include(CMakePrintHelpers)
cmake_minimum_required(VERSION 3.25)
project(WarFarmer)

option(NoGTest "NoGTest" OFF)
option(FlatPakHack "FlatPakHack" OFF)

add_subdirectory(netcode)


add_subdirectory(engine)
add_subdirectory(game)
if(NOT NoGTest)
    add_subdirectory(enginetest)
endif()
add_subdirectory(atlastool)
add_subdirectory(matchmaking_server)
add_subdirectory(ZzFX-C)

if(NOT WIN32)
    add_subdirectory(enginenettest)
endif()

target_link_libraries(WarFarmer PUBLIC StardewEngine)
if(NOT NoGTest)
    target_link_libraries(StardewEngineTest PUBLIC StardewEngine)
endif()

target_link_libraries(AtlasTool PUBLIC StardewEngine)

if(NOT WIN32)
    target_link_libraries(EngineNetTest PUBLIC StardewEngine)
endif()


if(WIN32)
# in windows we run cmake install to stage binaries including atlas tool then copy the assets at the end of the asset compilation stage
install(TARGETS WarFarmer StardewEngine AtlasTool StardewEngineTest DESTINATION WarFarmer)
else()
install(TARGETS WarFarmer DESTINATION bin)
install(TARGETS StardewEngine DESTINATION lib)
install(DIRECTORY WfAssets/
        DESTINATION share/WfAssets
        FILES_MATCHING
        PATTERN "*.txt"
        PATTERN "*.tilemap"
        PATTERN "*.atlas"
        PATTERN "*.xml"
        PATTERN "*.lua"
        PATTERN "*.game"
)
install(FILES WfAssets/ImageFiles.json DESTINATION share/WfAssets)
install(FILES WfAssets/Keymap.json DESTINATION share/WfAssets)
endif()