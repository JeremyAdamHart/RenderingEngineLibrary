#version 410

// location indices for these attributes correspond to those specified in the
// InitializeGeometry() function of the main program
layout(location = 0) in vec3 VertexPosition;

uniform mat4 model_view_matrix;
uniform mat4 projection_matrix;

out vec3 FragmentModelViewPosition;

void main()
{
    // assign vertex position without modification
    FragmentModelViewPosition = (model_view_matrix*vec4(VertexPosition, 1.0)).xyz;
    gl_Position = projection_matrix*vec4(FragmentModelViewPosition, 1.0);
}
