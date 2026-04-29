add_library(${PROJECT_NAME}.core STATIC
    downloader.cpp
    downloader.h
    base_music.cpp
    base_music.h
    qq_music.cpp
    qq_music.h
)
target_link_libraries(${PROJECT_NAME}.core PUBLIC ${LIBS})