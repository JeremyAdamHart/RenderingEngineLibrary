cmake_minimum_required(VERSION 3.20)

project(jorl VERSION 0.0.1)

add_library(jorl_core 
    ${CMAKE_CURRENT_SOURCE_DIR}/source/Angles.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/source/Camera.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/Drawable.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/Framebuffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/GLGeometry.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/glmSupport.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/GLObject.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/glSupport.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/Material.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/Object.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/Scene.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/Shader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/Texture.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/VertexAttribLayout.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/VertexBufferTemplates.cpp
)
add_library(jorl::core ALIAS jorl_core)

target_include_directories(jorl_core PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/jorl/core)
target_include_directories(jorl_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/)

# Packages
find_package(glfw3 REQUIRED)
find_package(glad REQUIRED)
find_package(glm REQUIRED)

target_link_libraries(jorl_core PUBLIC glfw)
target_link_libraries(jorl_core PUBLIC glm::glm)
target_link_libraries(jorl_core PUBLIC glad::glad)
