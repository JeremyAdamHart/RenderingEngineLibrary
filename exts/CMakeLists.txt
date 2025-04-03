cmake_minimum_required(VERSION 3.20)

add_library(jorl_exts
    ${CMAKE_CURRENT_SOURCE_DIR}/source/AOShader.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/source/Bitmask.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/source/BlendShader.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/source/BlinnPhongShader.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/source/ColorMat.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/source/ColorSetMat.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/source/ColorShader.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/source/CommonGeometry.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/source/GlowShader.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/source/MeshInfoLoader.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/source/ModelLoader.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/source/MultiBuffer.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/source/NormalDisplayShader.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/source/PerlinNoise.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/source/PosNormalShader.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/source/ProcessingShader.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/source/ReadMe.txt 
    ${CMAKE_CURRENT_SOURCE_DIR}/source/ShadedMat.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/source/ShadowShader.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/source/SimpleShader.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/source/SimpleTexManager.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/source/simpleTexShader.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/source/SphericalCamera.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/source/StreamGeometry.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/source/TemplatedGeometry.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/source/TextureCreation.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/source/TextureMat.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/source/TrackballCamera.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/source/TrackerCamera.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/FontToTexture.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/TextDrawable.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/TextShader.cpp
)
add_library(jorl::exts ALIAS jorl_exts)

target_include_directories(jorl_exts PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/jorl/exts)
target_include_directories(jorl_exts PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/)


# Packages
find_package(freetype REQUIRED)
find_package(stb REQUIRED)
find_package(tinyply REQUIRED)

target_link_libraries(jorl_core PUBLIC Freetype::Freetype)
target_link_libraries(jorl_exts PUBLIC jorl_core)
target_link_libraries(jorl_exts PUBLIC stb::stb)
target_link_libraries(jorl_exts PUBLIC tinyply::tinyply)