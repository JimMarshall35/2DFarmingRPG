include(CMakePrintHelpers)
cmake_minimum_required(VERSION 3.25)
project(WarFarmer)

add_subdirectory(engine)
add_subdirectory(game)

target_link_libraries(WarFarmer PUBLIC StardewEngine)


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