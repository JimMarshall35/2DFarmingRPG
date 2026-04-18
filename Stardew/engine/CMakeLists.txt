include(CMakePrintHelpers)
cmake_minimum_required(VERSION 3.25)
project(StardewEngine)

set(STARDEW_PLATFORM "GLFW3" CACHE STRING "Backend")

set_property(CACHE STARDEW_PLATFORM PROPERTY STRINGS
    GLFW3 SDL2
)

set(STARDEW_GL_API_TYPE "OPENGL" CACHE STRING "Backend")

set_property(CACHE STARDEW_GL_API_TYPE PROPERTY STRINGS
    OPENGL OPENGLES
)


if(STARDEW_PLATFORM STREQUAL "GLFW3")
  find_package(glfw3 REQUIRED)
elseif(STARDEW_PLATFORM STREQUAL "SDL2")
  find_package(SDL2 REQUIRED)
endif()

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

add_subdirectory(lib/box2d-3.1.1 EXCLUDE_FROM_ALL)

add_subdirectory(src)


target_link_libraries(StardewEngine PUBLIC
  cglm_headers)


if(STARDEW_PLATFORM STREQUAL "GLFW3")
  target_link_libraries(StardewEngine PUBLIC glfw)
elseif(STARDEW_PLATFORM STREQUAL "SDL2")
  if(WIN32)
    target_link_libraries(StardewEngine PUBLIC SDL2::SDL2 opengl32)
  else()
  
    target_link_libraries(StardewEngine PUBLIC SDL2::SDL2 GLESv2)
  endif()
endif()

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
  target_link_libraries(StardewEngine PUBLIC LibXml2::LibXml2 Freetype::Freetype lua::lua)
else()
  target_link_libraries(StardewEngine PUBLIC ${FREETYPE_LIBRARIES})
  target_link_libraries(StardewEngine PUBLIC m)
  target_link_libraries(StardewEngine PUBLIC PkgConfig::LUA)
  target_link_libraries(StardewEngine PUBLIC ${LIBXML2_LIBRARIES})
endif()

target_compile_definitions(StardewEngine PRIVATE STARDEW_EXPORTS)

if(STARDEW_PLATFORM STREQUAL "GLFW3")
    target_compile_definitions(StardewEngine PRIVATE STARDEW_PLATFORM=1)
elseif(STARDEW_PLATFORM STREQUAL "SDL2")
    target_compile_definitions(StardewEngine PRIVATE STARDEW_PLATFORM=2)
else()
    message(FATAL_ERROR "Unknown STARDEW_PLATFORM: ${STARDEW_PLATFORM}")
endif()


if(STARDEW_GL_API_TYPE STREQUAL "OPENGL")
  target_compile_definitions(StardewEngine PRIVATE GAME_GL_API_TYPE=2)
elseif(STARDEW_GL_API_TYPE STREQUAL "OPENGLES")
  target_compile_definitions(StardewEngine PRIVATE GAME_GL_API_TYPE=1)
endif()
