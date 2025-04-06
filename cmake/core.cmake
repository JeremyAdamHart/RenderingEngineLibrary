cmake_minimum_required(VERSION 3.20)

function(declare_jorl_core)
    add_library(jorl_core 
        core/source/Angles.cpp 
        core/source/Camera.cpp
        core/source/Drawable.cpp
        core/source/Framebuffer.cpp
        core/source/GLGeometry.cpp
        core/source/glmSupport.cpp
        core/source/GLObject.cpp
        core/source/glSupport.cpp
        core/source/Material.cpp
        core/source/Object.cpp
        core/source/Scene.cpp
        core/source/Shader.cpp
        core/source/Texture.cpp
        core/source/VertexAttribLayout.cpp
        core/source/VertexBufferTemplates.cpp
    )
    add_library(jorl::core ALIAS jorl_core)

    target_sources(jorl_core PUBLIC FILE_SET jorl_core_headers TYPE HEADERS BASE_DIRS ./include/ FILES    
        ./include/jorl/core/Angles.h 
        ./include/jorl/core/Camera.h 
        ./include/jorl/core/Drawable.h 
        ./include/jorl/core/Framebuffer.h 
        ./include/jorl/core/GLGeometry.h 
        ./include/jorl/core/glmSupport.h 
        ./include/jorl/core/GLObject.h 
        ./include/jorl/core/glSupport.h 
        ./include/jorl/core/Material.h 
        ./include/jorl/core/Object.h 
        ./include/jorl/core/PointerAliases.h 
        ./include/jorl/core/Scene.h 
        ./include/jorl/core/Shader.h 
        ./include/jorl/core/Texture.h 
        ./include/jorl/core/VertexAttribLayout.h 
        ./include/jorl/core/VertexBufferTemplates.h
    )

    target_include_directories(jorl_core PRIVATE ./include/jorl/core)
    target_include_directories(jorl_core PUBLIC ./include/)

    # set_target_properties(jorl_core PROPERTIES PUBLIC_HEADER ./include/jorl/core/Angles.h)

    # Packages
    find_package(glfw3 REQUIRED)
    find_package(glad REQUIRED)
    find_package(glm REQUIRED)

    target_link_libraries(jorl_core PUBLIC glfw)
    target_link_libraries(jorl_core PUBLIC glm::glm)
    target_link_libraries(jorl_core PUBLIC glad::glad)
endfunction()

# install(TARGETS jorl_core FILE_SET jorl_core_headers)

# install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION "")

