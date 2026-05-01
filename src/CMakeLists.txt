# 先添加io子目录
add_subdirectory(io)
add_subdirectory(core)

add_executable(${PROJECT_NAME} WIN32
    main.cpp
    main_window.h
    main_window.cpp
    music_window.h
    music_window.cpp
    resource/app.rc
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${LIBS}
    ${PROJECT_NAME}.io
    ${PROJECT_NAME}.core
)

qt_add_resources(${PROJECT_NAME} "LCX"
    RESOURCES PREFIX "/"
    FILES
    resource/liu.jpg
)