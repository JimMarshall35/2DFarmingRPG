add_executable(MatchmakingServer 
main.c
)

target_link_libraries(MatchmakingServer PUBLIC Netcode)

target_include_directories(MatchmakingServer PRIVATE ../netcode)