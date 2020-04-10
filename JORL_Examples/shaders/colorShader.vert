#version 450

// location indices for these attributes correspond to those specified in the
// InitializeGeometry() function of the main program
layout(location = 0) in vec3 VertexPosition;
layout(location = 1) in vec3 VertexNormal;
layout(location = 2) in int VertexColorIndex;

uniform mat4 view_projection_matrix;
uniform mat4 model_matrix;

out vec3 FragmentColor;
out vec3 WorldPosition;
out vec3 WorldNormal;

//uniform vec3 colorA = vec3(1, 0, 0);
//uniform vec3 colorB = vec3(0, 0, 1);
uniform vec3 colors [MAX_COLOR_NUM];
const vec3 otherColors[2] = vec3[2](vec3(1, 0, 0), vec3(0, 1, 0));

void main()
{
	WorldNormal = (model_matrix*vec4(VertexNormal, 0.0)).xyz;
	WorldPosition = (model_matrix*vec4(VertexPosition, 1.0)).xyz;

	FragmentColor = colors[VertexColorIndex];
    // assign vertex position without modification
    gl_Position = view_projection_matrix*model_matrix*vec4(VertexPosition, 1.0);
}
