include(CMakePrintHelpers)
cmake_minimum_required(VERSION 3.16)
project(WarFarmer)

add_subdirectory(engine)
add_subdirectory(game)

target_link_libraries(WarFarmer PUBLIC StardewEngine)


if(WIN32)
        # in windows we run cmake install to stage binaries including atlas tool then copy the assets at the end of the asset compilation stage
        install(TARGETS WarFarmer StardewEngine AtlasTool StardewEngineTest DESTINATION WarFarmer)
else()
        include(GNUInstallDirs)
        install(TARGETS WarFarmer DESTINATION ${CMAKE_INSTALL_BINDIR})
        install(TARGETS StardewEngine DESTINATION ${CMAKE_INSTALL_LIBDIR})
        install(DIRECTORY WfAssets/
                DESTINATION ${CMAKE_INSTALL_DATADIR}/WfAssets
                FILES_MATCHING
                PATTERN "*.txt"
                PATTERN "*.tilemap"
                PATTERN "*.atlas"
                PATTERN "*.xml"
                PATTERN "*.lua"
                PATTERN "*.game"
        )
        install(FILES WfAssets/Keymap.json DESTINATION ${CMAKE_INSTALL_DATADIR}/WfAssets)
endif()