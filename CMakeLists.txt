cmake_minimum_required(VERSION 3.16.3)

# set(CMAKE_VERBOSE_MAKEFILE ON)

include(cmake/utils.cmake)

project(game_engine)

add_definitions("-Wall" "-g" "-O3" "-std=c++20" "-fconcepts-diagnostics-depth=2")
include_directories(include)
include_directories(extern)
include_directories(extern/shaderc/src/libshaderc/include)
link_directories(extern)
link_directories(extern/shaderc/libshaderc)
link_directories(${CMAKE_BINARY_DIR}/src)

set_property(GLOBAL PROPERTY CXX_STANDARD 20)
set_property(GLOBAL PROPERTY CXX_STANDARD_REQUIRED)

link_libraries(-lglfw -lvulkan)

add_subdirectory(src)
add_subdirectory(tests)
