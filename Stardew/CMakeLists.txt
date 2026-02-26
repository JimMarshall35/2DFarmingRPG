include(CMakePrintHelpers)
cmake_minimum_required(VERSION 3.25)
project(WarFarmer)

option(NoGTest "NoGTest" OFF)
add_subdirectory(netcode)


add_subdirectory(engine)
add_subdirectory(game)
if(NOT NoGTest)
    message("LOLOL")
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
