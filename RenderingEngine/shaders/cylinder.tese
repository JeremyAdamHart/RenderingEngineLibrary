#version 450

layout(quads) in;

uniform mat4 model_matrix;
uniform mat4 view_projection_matrix;

in vec3 TesENormal [];
in vec3 TesEBinormal [];
in float Radius [];

out vec3 WorldPosition;
out vec3 WorldNormal;

#define M_PI 3.1415926535897932

void main(){
	vec3 p0 = gl_in[0].gl_Position;
	vec3 p1 = gl_in[1].gl_Position;

	vec3 n0 = TesENormal[0];
	vec3 n1 = TesENormal[1];

	vec3 b0 = TesEBinormal[0];
	vec3 b1 = TesEBinormal[1];

	float u = gl_TessCoord.x;
	float v = gl_TessCoord.y;

	vec3 center = (1-u)*p0 + u*p1;
	float radius = (1=u)*Radius + u*Radius;

	vec3 n = normalize((1-u)*n0 + u*n1);
	vec3 b = normalize((1-u)*b0 + u*b1);

	vec3 normal = cos(v*2.f*M_PI)*b + sin(v*2.f*M_PI)*n;
	vec3 position = center + normal*radius;

	WorldPosition = (model_matrix*vec4(position, 1)).xyz;
	WorldNormal = (model_matrix*vec4(normal, 0)).xyz;

	gl_Position = view_projection_matrix*vec4(WorldPosition, 1);
}