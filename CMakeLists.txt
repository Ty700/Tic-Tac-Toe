cmake_minimum_required(VERSION 3.28)
project(TicTacToe VERSION 1.0.0)

set(EXE_NAME TicTacToe)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(${EXE_NAME} src/TicTacToe.cpp)
add_subdirectory(src)
add_subdirectory(includes)

target_compile_options(${EXE_NAME} PRIVATE -Wall -Wextra)

target_compile_options(${EXE_NAME} PRIVATE
  $<$<CONFIG:Debug>:-DDEBUG>
)

set_target_properties(${EXE_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/../bin/debug"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/../bin/release"
)
