include(CMakePrintHelpers)
cmake_minimum_required(VERSION 3.16)
project(WarFarmer)

option(NoGTest "NoGTest" OFF)
option(FlatPakHack "FlatPakHack" OFF)
option(ENABLE_PROFILING "Enable Tracy profiling" OFF)

add_subdirectory(netcode)


add_subdirectory(engine)
if(NOT NoGTest)
    add_subdirectory(enginetest)
endif()
add_subdirectory(atlastool)
add_subdirectory(ZzFX-C)

if(NOT NoGTest)
    target_link_libraries(StardewEngineTest PUBLIC StardewEngine)
endif()

target_link_libraries(AtlasTool PUBLIC StardewEngine)



if(WIN32)
# in windows we run cmake install to stage binaries including atlas tool then copy the assets at the end of the asset compilation stage
install(TARGETS StardewEngine StardewEngineTest DESTINATION Tests)
endif()

