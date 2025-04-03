#version 450

layout(location = 0) in vec3 VertexPosition;
layout(location = 1) in vec3 VertexVector;	//Vector on cap of cylinder -- Magnitude is the radius of the cylinder at that end

out vec3 TesCNormal;

void main(){
	gl_Position = vec4(VertexPosition, 1);
	TesCNormal = VertexVector;

}