add_executable(main
    main.cpp
    main_window.cpp
    main_window.h
    main_window.ui
    ${CMAKE_SOURCE_DIR}/resource/resources.qrc
    ${CMAKE_SOURCE_DIR}/resource/app.rc
)
target_link_libraries(main PRIVATE
    ${PROJECT_NAME}.core
    ${third_party_libs}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(main)
endif()