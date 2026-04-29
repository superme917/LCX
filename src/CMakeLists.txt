# 先添加io子目录
add_subdirectory(io)

qt_add_executable(${PROJECT_NAME}
    MANUAL_FINALIZATION
    main.cpp
    mainwindow.h
    mainwindow.cpp
)

# 确保LCX.io先构建
add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}.io)

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${LIBS}
    ${PROJECT_NAME}.io
)

qt_add_resources(${PROJECT_NAME} "LCX"
    RESOURCES PREFIX "/"
    FILES
    ../resource/liu.jpg
)