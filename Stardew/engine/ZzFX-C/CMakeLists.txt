include(CMakePrintHelpers)
cmake_minimum_required(VERSION 3.25)
project(ZzFX_C)

add_subdirectory(core)
add_subdirectory(openal_backend)

target_link_libraries(ZzFX_OpenAL PUBLIC ZzFX)