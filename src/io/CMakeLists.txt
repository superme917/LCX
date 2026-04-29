add_library(${PROJECT_NAME}.io STATIC
    word_engine.h
    word_engine.cpp
    word_export.h
    word_export.cpp
)
target_link_libraries(${PROJECT_NAME}.io PUBLIC ${LIBS})