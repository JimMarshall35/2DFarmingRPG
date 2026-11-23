set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mavx2")

add_library( ZzFX STATIC
    src/ZzFX.c
)
target_include_directories(ZzFX  PUBLIC include)
