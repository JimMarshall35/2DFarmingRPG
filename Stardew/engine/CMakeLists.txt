include(CMakePrintHelpers)
cmake_minimum_required(VERSION 3.25)
project(StardewEngine)

option(NO_OPENAL "Don't include openAL, for containerized build for server for examle" OFF)

find_package(glfw3 REQUIRED)
find_package(Lua 5.4 REQUIRED)
find_package(Freetype REQUIRED)
find_package(LibXml2 REQUIRED)

add_subdirectory(lib/box2d-3.1.1)


add_subdirectory(src)


target_link_libraries(StardewEngine PUBLIC
  cglm_headers)


target_link_libraries(StardewEngine PUBLIC glfw)

target_link_libraries(StardewEngine PUBLIC box2d)

target_link_libraries(StardewEngine PUBLIC Netcode)


add_subdirectory(lib/cglm-0.9.6 EXCLUDE_FROM_ALL)

if(NOT NO_OPENAL)
  if(NOT WIN32)
    find_package(OpenAL REQUIRED)
    target_link_libraries(StardewEngine PUBLIC OpenAL::OpenAL)
  else()
    add_subdirectory(lib/openal-soft-1.25.1)
    target_link_libraries(StardewEngine PUBLIC OpenAL)
    target_include_directories(StardewEngine  PRIVATE lib/openal-soft-1.25.1/include)
  endif()
endif()

target_include_directories(StardewEngine  PUBLIC include)
target_include_directories(StardewEngine  PRIVATE lib/glad/include)
target_include_directories(StardewEngine  PRIVATE ../netcode)
target_include_directories(StardewEngine  PRIVATE ../netcode/sodium)


set_property(TARGET StardewEngine PROPERTY C_STANDARD 11)

target_include_directories(StardewEngine PUBLIC ${FREETYPE_INCLUDE_DIRS})
target_include_directories(StardewEngine PUBLIC ${LUA_INCLUDE_DIR})
target_include_directories(StardewEngine PUBLIC ${LIBXML2_INCLUDE_DIR})

if(WIN32)
  target_link_libraries(StardewEngine PUBLIC LibXml2::LibXml2 Freetype::Freetype lua::lua glfw)
else()
  target_link_libraries(StardewEngine PUBLIC ${FREETYPE_LIBRARIES})
  target_link_libraries(StardewEngine PUBLIC ${LUA_LIBRARIES})
  target_link_libraries(StardewEngine PUBLIC ${LIBXML2_LIBRARIES})
endif()