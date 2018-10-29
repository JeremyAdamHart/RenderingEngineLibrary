#version 450

layout(location = 0) in vec3 VertexPosition;
layout(location = 1) in vec3 VertexNormal;

out vec3 TesCNormal;

void main(){
	gl_Position = VertexPosition;
	TesCNormal = VertexNormal;

}