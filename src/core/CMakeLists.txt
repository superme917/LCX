add_library(${PROJECT_NAME}.core STATIC
    process/server_process.cpp
    process/server_process.h
    process/network_request.cpp
    process/network_request.h
    music_interface/qq_music_interface.cpp
    music_interface/qq_music_interface.h
    music_interface/base_music_interface.h
)
target_link_libraries(${PROJECT_NAME}.core PUBLIC ${third_party_libs})
