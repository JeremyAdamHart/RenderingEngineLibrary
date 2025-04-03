#version 410

// location indices for these attributes correspond to those specified in the
// InitializeGeometry() function of the main program
layout(location = 0) in vec3 VertexPosition;

uniform mat4 view_projection_matrix;
uniform mat4 model_matrix;

out vec4 ModelPosition;

void main()
{
    // assign vertex position without modification
    ModelPosition = model_matrix*vec4(VertexPosition, 1.0);
    gl_Position = view_projection_matrix*model_matrix*vec4(VertexPosition, 1.0);
}
