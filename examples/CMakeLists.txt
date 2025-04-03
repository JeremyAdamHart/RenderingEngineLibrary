cmake_minimum_required(VERSION 3.20)

add_executable(jorl_examples
    ${CMAKE_CURRENT_SOURCE_DIR}/JORL_Examples.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/AdaptiveNoise.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/GLWindow.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/HeatParticleGeometry.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/HeatParticleMat.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/HeatParticleShader.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/HeatParticleSystem.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/noiseTest.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/PetioleAlignment.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/Physics.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/VertexVertex.cpp
)

# Packages
target_link_libraries(jorl_examples PUBLIC jorl_exts)
