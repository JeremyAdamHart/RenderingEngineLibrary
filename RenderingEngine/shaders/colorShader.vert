#version 410

// location indices for these attributes correspond to those specified in the
// InitializeGeometry() function of the main program
layout(location = 0) in vec3 VertexPosition;
layout(location = 1) in vec3 VertexNormal;
layout(location = 2) in int VertexColor;

uniform mat4 view_projection_matrix;
uniform mat4 model_matrix;

flat out int FragmentColor;
out vec3 WorldPosition;
out vec3 WorldNormal;

void main()
{
	WorldNormal = (model_matrix*vec4(VertexNormal, 0.0)).xyz;
	WorldPosition = (model_matrix*vec4(VertexPosition, 1.0)).xyz;
	FragmentColor = VertexColor;
    // assign vertex position without modification
    gl_Position = view_projection_matrix*model_matrix*vec4(VertexPosition, 1.0);
}
