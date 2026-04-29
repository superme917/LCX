add_library(${PROJECT_NAME}.io STATIC
    word_engine.h
    word_engine.cpp
)
target_link_libraries(${PROJECT_NAME}.io PUBLIC ${LIBS})