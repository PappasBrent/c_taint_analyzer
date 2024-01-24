cmake_minimum_required(VERSION 3.28.1)
project(c_taint_analyzer
        VERSION 0.0.0
        DESCRIPTION "A plugin for Clang compiler."
        LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Werror")

find_package(Clang 17 REQUIRED CONFIG)
find_package(LLVM 17 REQUIRED CONFIG)

add_subdirectory(lib)