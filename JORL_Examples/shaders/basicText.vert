#version 410

// location indices for these attributes correspond to those specified in the
// InitializeGeometry() function of the main program
layout(location = 0) in vec3 VertexPosition;
layout(location = 2) in vec2 VertexTexCoord;

uniform mat4 model_view_projection_matrix;

out vec2 FragmentTexCoord;

void main()
{
    // assign vertex position without modification
    FragmentTexCoord = VertexTexCoord;
    gl_Position = model_view_projection_matrix*vec4(VertexPosition, 1.0);
}
