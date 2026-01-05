include(CMakePrintHelpers)
cmake_minimum_required(VERSION 3.25)
project(StardewEngine)

if(WIN32)
find_package(glfw3 CONFIG REQUIRED)
find_package(lua CONFIG REQUIRED)
find_package(freetype CONFIG REQUIRED)
find_package(libxml2 CONFIG REQUIRED)
else()
find_package(glfw3 REQUIRED)
find_package(Lua 5.4 REQUIRED)
find_package(Freetype REQUIRED)
find_package(LibXml2 REQUIRED)

endif()

add_subdirectory(lib/box2d-3.1.1)


if(Lua_FOUND AND NOT TARGET Lua::Lua)
  add_library(Lua::Lua INTERFACE IMPORTED)
  set_target_properties(
    Lua::Lua
    PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${LUA_INCLUDE_DIR}"
      INTERFACE_LINK_LIBRARIES "${LUA_LIBRARIES}"
  )
endif()

add_subdirectory(src)


target_link_libraries(StardewEngine PUBLIC
  cglm_headers)


target_link_libraries(StardewEngine PUBLIC glfw)

target_link_libraries(StardewEngine PUBLIC box2d)

target_link_libraries(StardewEngine PUBLIC Netcode)


add_subdirectory(lib/cglm-0.9.6 EXCLUDE_FROM_ALL)


target_include_directories(StardewEngine  PUBLIC include)
target_include_directories(StardewEngine  PRIVATE lib/glad/include)
target_include_directories(StardewEngine  PRIVATE ../netcode)
target_include_directories(StardewEngine  PRIVATE ../netcode/sodium)

set_property(TARGET StardewEngine PROPERTY C_STANDARD 11)

# target_include_directories(NetcodeExampleServer  PRIVATE src/vendor/netcode-1.2.3)
# target_include_directories(NetcodeExampleServer  PRIVATE src/vendor/netcode-1.2.3/sodium)

# target_include_directories(NetcodeExampleClient  PRIVATE src/vendor/netcode-1.2.3)
# target_include_directories(NetcodeExampleClient  PRIVATE src/vendor/netcode-1.2.3/sodium)

if(WIN32)
else()
  target_include_directories(StardewEngine PUBLIC ${FREETYPE_INCLUDE_DIRS})
  target_include_directories(StardewEngine PUBLIC ${LUA_INCLUDE_DIR})
  target_include_directories(StardewEngine PUBLIC ${LIBXML2_INCLUDE_DIR})

 endif()

if(WIN32)
  target_link_libraries(StardewEngine PUBLIC LibXml2::LibXml2 Freetype::Freetype lua::lua glfw)
else()
  target_link_libraries(StardewEngine PUBLIC ${FREETYPE_LIBRARIES})
  target_link_libraries(StardewEngine PUBLIC ${LUA_LIBRARIES})
  target_link_libraries(StardewEngine PUBLIC ${LIBXML2_LIBRARIES})
  target_link_libraries(StardewEngine PUBLIC Lua::Lua) 
endif()

