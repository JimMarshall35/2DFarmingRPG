add_executable( EngineNetTest main.c cwalk.c )
target_include_directories(EngineNetTest PRIVATE .)
target_include_directories(EngineNetTest PRIVATE ../netcode)
