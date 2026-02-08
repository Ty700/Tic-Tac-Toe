cmake_minimum_required(VERSION 3.28)
project(TicTacToe VERSION 1.0.0)
include(ProcessorCount)

ProcessorCount(N)
if(NOT N EQUAL 0)
	set(CMAKE_BUILD_PARALLEL_LEVEL ${N})
	message(STATUS "Building with ${N} Jobs")
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM REQUIRED gtkmm-4.0)
find_package(OpenSSL REQUIRED)

set(EXE_NAME TicTacToe)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(${EXE_NAME} src/TicTacToe.cpp)
add_subdirectory(src)
add_subdirectory(includes)
include_directories(${GTKMM_INCLUDE_DIRS})

target_compile_options(${EXE_NAME} PRIVATE -Wall -Wextra ${GTK4_CFLAGS})
target_include_directories(${EXE_NAME} PRIVATE ${GTKMM_INCLUDE_DIRS})
target_link_libraries(${EXE_NAME} PRIVATE ${GTKMM_LIBRARIES} OpenSSL::SSL OpenSSL::Crypto)

target_compile_options(${EXE_NAME} PRIVATE
  $<$<CONFIG:Debug>:-DDEBUG>
)

set_target_properties(${EXE_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/../bin/debug"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/../bin/release"
)
