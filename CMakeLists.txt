cmake_minimum_required(VERSION 3.20)

project(jorl VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "") # works

add_subdirectory(core)
add_subdirectory(exts)
add_subdirectory(examples)