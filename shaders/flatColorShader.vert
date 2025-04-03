#version 410

layout(location =0) in vec3 VertexPosition;

uniform mat4 ModelViewProjectionMatrix;

void main()
{
	gl_Position = ModelViewProjectionMatrix*vec4(VertexPosition, 1);
}