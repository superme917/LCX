add_library(${PROJECT_NAME}.io STATIC
    word_engine.h
    word_engine.cpp
    word_export.h
    word_export.cpp
    export_wait_dialog.h
    export_wait_dialog.cpp
)
target_link_libraries(${PROJECT_NAME}.io PUBLIC ${LIBS})