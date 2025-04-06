cmake_minimum_required(VERSION 3.20)

project(jorl VERSION 0.0.1)

include(cmake/core.cmake)
include(cmake/exts.cmake)

IF (WIN32)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" "/DWIN32_LEAN_AND_MEAN /DNOSERVICE /DNOMCX /DNOIME /DNOMINMAX /permissive- /volatile:iso /Zc:preprocessor /Zc:__cplusplus /Zc:externConstexpr /Zc:throwingNew /EHsc" PARENT_SCOPE)
ENDIF()

# add_compile_definitions(USE_GL_4_2)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "") # works

declare_jorl_core()
declare_jorl_exts()

get_target_property(CORE_INCLS jorl_core INCLUDE_DIRECTORIES)
get_target_property(EXTS_INCLS jorl_exts INCLUDE_DIRECTORIES)


message("----------------------------------------")
message(${CORE_INCLS})
message("----------------------------------------")
message(${EXTS_INCLS})

install(TARGETS jorl_core jorl_exts FILE_SET jorl_core_headers FILE_SET jorl_exts_headers)

# add_subdirectory(core)
# add_subdirectory(exts)

# install(TARGETS jorl_core jorl_exts)
# add_subdirectory(examples)
# add_subdirectory(vr)