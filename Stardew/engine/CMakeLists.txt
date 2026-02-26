include(CMakePrintHelpers)
cmake_minimum_required(VERSION 3.25)
project(StardewEngine)

find_package(glfw3 REQUIRED)

if(WIN32)
  find_package(Lua 5.4 REQUIRED)
else()
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(LUA REQUIRED IMPORTED_TARGET lua5.4)
endif()

find_package(Freetype REQUIRED)
find_package(LibXml2 REQUIRED)
find_package(OpenAL REQUIRED)

message("lua version: ${Lua_VERSION}")
find_package(Lua 5.4 REQUIRED)

add_subdirectory(lib/box2d-3.1.1)


add_subdirectory(src)


target_link_libraries(StardewEngine PUBLIC
  cglm_headers)


target_link_libraries(StardewEngine PUBLIC glfw)

target_link_libraries(StardewEngine PUBLIC box2d)

target_link_libraries(StardewEngine PUBLIC Netcode)

target_link_libraries(StardewEngine PUBLIC ZzFX)

add_subdirectory(lib/cglm-0.9.6 EXCLUDE_FROM_ALL)


target_include_directories(StardewEngine  PUBLIC include)
target_include_directories(StardewEngine  PRIVATE lib/glad/include)
target_include_directories(StardewEngine  PRIVATE ../netcode)
target_include_directories(StardewEngine  PRIVATE ../netcode/sodium)
target_include_directories(StardewEngine  PRIVATE ../ZzFX-C/core/include)


set_property(TARGET StardewEngine PROPERTY C_STANDARD 11)

target_include_directories(StardewEngine PUBLIC ${FREETYPE_INCLUDE_DIRS})
target_include_directories(StardewEngine PUBLIC ${LUA_INCLUDE_DIR})
target_include_directories(StardewEngine PUBLIC ${LIBXML2_INCLUDE_DIR})
target_include_directories(StardewEngine PUBLIC ${LIBOPENAL_INCLUDE_DIR})

target_link_libraries(StardewEngine PUBLIC OpenAL::OpenAL)
if(WIN32)
  target_link_libraries(StardewEngine PUBLIC LibXml2::LibXml2 Freetype::Freetype lua::lua glfw)
else()
  target_link_libraries(StardewEngine PUBLIC ${FREETYPE_LIBRARIES})
  target_link_libraries(StardewEngine PUBLIC m)
  target_link_libraries(StardewEngine PUBLIC PkgConfig::LUA)
  target_link_libraries(StardewEngine PUBLIC ${LIBXML2_LIBRARIES})
endif()
