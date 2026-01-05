add_executable(MatchmakingServer 
main.cpp
)

add_executable(EchoClient 
client.cpp
)


target_link_libraries(MatchmakingServer PUBLIC Netcode)

target_include_directories(MatchmakingServer PRIVATE ../netcode)