project(PieceTableTests)


add_executable(
    piece_table
    piece_table.cpp
)

target_include_directories(piece_table PRIVATE ${CMAKE_SOURCE_DIR}/src/piece_table)

target_link_libraries(
    piece_table
    PRIVATE
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(piece_table)
