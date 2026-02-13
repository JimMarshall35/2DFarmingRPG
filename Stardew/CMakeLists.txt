include(CMakePrintHelpers)
cmake_minimum_required(VERSION 3.25)
project(WarFarmer)

add_subdirectory(netcode)


add_subdirectory(engine)
add_subdirectory(game)
add_subdirectory(enginetest)
add_subdirectory(atlastool)
add_subdirectory(matchmaking_server)
add_subdirectory(ZzFX-C)

if(NOT WIN32)
    add_subdirectory(enginenettest)
endif()

target_link_libraries(WarFarmer PUBLIC StardewEngine)
target_link_libraries(StardewEngineTest PUBLIC StardewEngine)
target_link_libraries(AtlasTool PUBLIC StardewEngine)
target_link_libraries(ZzFX PUBLIC StardewEngine)

if(NOT WIN32)
    target_link_libraries(EngineNetTest PUBLIC StardewEngine)
endif()
