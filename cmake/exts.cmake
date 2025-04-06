
function(declare_jorl_exts)
    add_library(jorl_exts
        exts/source/AOShader.cpp 
        exts/source/Bitmask.cpp 
        exts/source/BlendShader.cpp 
        exts/source/BlinnPhongShader.cpp 
        exts/source/ColorMat.cpp 
        exts/source/ColorSetMat.cpp 
        exts/source/ColorShader.cpp 
        exts/source/CommonGeometry.cpp 
        exts/source/GlowShader.cpp 
        exts/source/MeshInfoLoader.cpp 
        exts/source/ModelLoader.cpp 
        exts/source/MultiBuffer.cpp 
        exts/source/NormalDisplayShader.cpp 
        exts/source/PerlinNoise.cpp 
        exts/source/PosNormalShader.cpp 
        exts/source/ProcessingShader.cpp 
        exts/source/ReadMe.txt 
        exts/source/ShadedMat.cpp 
        exts/source/ShadowShader.cpp 
        exts/source/SimpleShader.cpp 
        exts/source/SimpleTexManager.cpp 
        exts/source/simpleTexShader.cpp 
        exts/source/SphericalCamera.cpp 
        exts/source/StreamGeometry.cpp 
        exts/source/TemplatedGeometry.cpp 
        exts/source/TextureCreation.cpp 
        exts/source/TextureMat.cpp 
        exts/source/TrackballCamera.cpp 
        exts/source/TrackerCamera.cpp
        exts/source/FontToTexture.cpp
        exts/source/TextDrawable.cpp
        exts/source/TextShader.cpp
    )
    add_library(jorl::exts ALIAS jorl_exts)

    target_sources(jorl_exts PUBLIC FILE_SET jorl_exts_headers TYPE HEADERS BASE_DIRS ./include/ FILES
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
endfunction()

# install(TARGETS jorl_exts)

# install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION "")
