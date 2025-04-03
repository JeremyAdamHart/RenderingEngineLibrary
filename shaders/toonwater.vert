#version 410

// location indices for these attributes correspond to those specified in the
// InitializeGeometry() function of the main program
layout(location = 0) in vec3 VertexPosition;

uniform mat4 model_matrix;


void main()
{
    // assign vertex position without modification
    gl_Position = model_matrix*vec4(VertexPosition, 1.0);
}
