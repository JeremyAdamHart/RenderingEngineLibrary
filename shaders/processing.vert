#version 410

layout(location=0) vec3 VertexPosition;
layout(location=2) ivec2 VertexTexCoord;

out ivec2 FragmentTexCoord

void main(){
	FragmentTexCoord = VertexTexCoord;
}