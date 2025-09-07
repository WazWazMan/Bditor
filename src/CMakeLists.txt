project(main VERSION 0.1.0 LANGUAGES C CXX)

add_executable(${PROJECT_NAME} main.cpp)

add_subdirectory(piece_table)

target_include_directories(${PROJECT_NAME} PUBLIC piece_table)
target_link_libraries(main PUBLIC PieceTable)