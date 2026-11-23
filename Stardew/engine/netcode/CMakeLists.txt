file(GLOB SODIUM_SRC ./sodium/*.c)


add_library( Netcode STATIC 
netcode.c
${SODIUM_SRC}
)

add_executable( TestClient 
client.c 
netcode.c
${SODIUM_SRC}
)

target_include_directories(Netcode  PUBLIC ./sodium)
target_include_directories(TestClient  PUBLIC ./sodium)

if(NOT WIN32)
    target_compile_options(Netcode PUBLIC -fPIC)
endif()
