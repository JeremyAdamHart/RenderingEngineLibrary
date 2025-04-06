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

target_sources(jorl_exts PUBLIC FILE_SET HEADERS FILES 
    ./include/jorl/exts/AOShader.h 
    ./include/jorl/exts/Bitmask.h 
    ./include/jorl/exts/BlendShader.h 
    ./include/jorl/exts/BlinnPhongShader.h 
    ./include/jorl/exts/BufferQueue.h 
    ./include/jorl/exts/ColorMat.h 
    ./include/jorl/exts/ColorSetMat.h 
    ./include/jorl/exts/ColorShader.h 
    ./include/jorl/exts/CommonGeometry.h 
    ./include/jorl/exts/ConvexHull.h 
    ./include/jorl/exts/FontToTexture.h 
    ./include/jorl/exts/GlowShader.h 
    ./include/jorl/exts/MeshInfoLoader.h 
    ./include/jorl/exts/ModelLoader.h 
    ./include/jorl/exts/MTGeometry.h 
    ./include/jorl/exts/MultiBuffer.h 
    ./include/jorl/exts/MultiThreadedResource.h 
    ./include/jorl/exts/NormalDisplayShader.h 
    ./include/jorl/exts/PerlinNoise.h 
    ./include/jorl/exts/PosNormalShader.h 
    ./include/jorl/exts/ProcessingShader.h 
    ./include/jorl/exts/ShadedMat.h 
    ./include/jorl/exts/ShaderCompiler.h 
    ./include/jorl/exts/ShadowShader.h 
    ./include/jorl/exts/SimpleShader.h 
    ./include/jorl/exts/SimpleTexManager.h 
    ./include/jorl/exts/simpleTexShader.h 
    ./include/jorl/exts/SphericalCamera.h 
    ./include/jorl/exts/StreamGeometry.h 
    ./include/jorl/exts/TemplatedGeometry.h 
    ./include/jorl/exts/TemplatedShader.h 
    ./include/jorl/exts/TemplateParameterParsing.h 
    ./include/jorl/exts/TextShader.h 
    ./include/jorl/exts/TextureCreation.h 
    ./include/jorl/exts/TextureMat.h 
    ./include/jorl/exts/TrackballCamera.h 
    ./include/jorl/exts/TrackerCamera.h 
    ./include/jorl/exts/VertexAttributeTemplates.h
)

target_include_directories(jorl_exts PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/jorl/exts)
target_include_directories(jorl_exts PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/)


# Packages
find_package(freetype REQUIRED)
find_package(stb REQUIRED)
find_package(tinyply REQUIRED)

target_link_libraries(jorl_exts PUBLIC Freetype::Freetype)
target_link_libraries(jorl_exts PUBLIC jorl_core)
target_link_libraries(jorl_exts PUBLIC stb::stb)
target_link_libraries(jorl_exts PUBLIC tinyply::tinyply)

install(TARGETS jorl_exts)

# install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION "")
