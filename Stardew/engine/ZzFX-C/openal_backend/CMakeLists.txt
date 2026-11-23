add_library( ZzFX_OpenAL SHARED
    src/openAL.c
    src/ObjectPool.c
)
target_include_directories(ZzFX_OpenAL  PUBLIC include)
target_include_directories(ZzFX_OpenAL PUBLIC ../core/include)


find_package(OpenAL REQUIRED)
target_include_directories(ZzFX_OpenAL PUBLIC ${LIBOPENAL_INCLUDE_DIR})
target_link_libraries(ZzFX_OpenAL PUBLIC OpenAL::OpenAL)


