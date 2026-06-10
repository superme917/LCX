add_library(${PROJECT_NAME}.core STATIC
    base_music.cpp
    base_music.h
    qq_music.cpp
    qq_music.h
    cloud_music.cpp
    cloud_music.h
    kugou_music.cpp
    kugou_music.h
    qishui_music.cpp
    qishui_music.h
    deepseek_client.cpp
    deepseek_client.h
    deepseek_lyric_cleaner.cpp
    deepseek_lyric_cleaner.h
)
target_link_libraries(${PROJECT_NAME}.core PUBLIC ${LIBS})