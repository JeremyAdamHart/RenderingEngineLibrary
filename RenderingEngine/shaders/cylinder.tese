#version 450

layout(quads) in;

uniform mat4 model_matrix;
uniform mat4 view_projection_matrix;

in vec3 TesENormal [];
in vec3 TesEBinormal [];
in vec3 TesENormal_Par [];
in float Radius [];

out vec3 WorldPosition;
out vec3 WorldNormal;

#define M_PI 3.1415926535897932

vec3 slerp(vec3 a, vec3 b, float u){
	float angle = acos(dot(a, b));
	return (sin((1-u)*angle)*a + sin(u*angle)*b)/sin(angle);
}



void main(){
	
	vec3 p0 = gl_in[0].gl_Position.xyz;
	vec3 p1 = gl_in[1].gl_Position.xyz;

	vec3 dir = normalize(p1 - p0);

	vec3 n0 = TesENormal[0];
	vec3 n1 = TesENormal[1];

	vec3 b0 = TesEBinormal[0];
	vec3 b1 = TesEBinormal[1];

	vec3 n_p0 = TesENormal_Par[0];
	vec3 n_p1 = TesENormal_Par[1]; 

	float u = gl_TessCoord.x;
	float v = gl_TessCoord.y;

	vec3 center = (1-u)*p0 + u*p1;
	float radius = (1-u)*Radius[0] + u*Radius[1];

	vec3 n_p = (1-u)*n_p0 + u*n_p1;
	vec3 n = slerp(n0, n1, u);	//normalize((1-u)*n0 + u*n1);
	vec3 b = slerp(b0, b1, u);	//normalize((1-u)*b0 + u*b1);

	vec3 normal = cos(v*2.f*M_PI)*b + sin(v*2.f*M_PI)*(n_p + n);
	vec3 position = center + normal*radius;

	//Normal
	/*float x = Radius[1] - Radius[0];
	float y = length(p1-p0);
	float xyLength = sqrt(x*x+y*y);
	x /= xyLength;
	y /= xyLength;

	
	vec3 cNormal = y*normalize(cos(v*2.f*M_PI)*b + sin(v*2.f*M_PI)*n)
		+ x*normalize(p0-p1);
	*/

	vec3 cNormal =normalize(cos(v*2.f*M_PI)*b + sin(v*2.f*M_PI)*n);

	WorldPosition = (model_matrix*vec4(position, 1)).xyz;
	WorldNormal = (model_matrix*vec4(cNormal, 0)).xyz;

	gl_Position = view_projection_matrix*vec4(WorldPosition, 1);

	/*
	vec3 p0 = gl_in[0].gl_Position.xyz;
	vec3 p1 = gl_in[1].gl_Position.xyz;

	vec3 dir = normalize(p1 - p0);

	vec3 n0 = TesENormal[0];
	vec3 n1 = TesENormal[1];

	vec3 b0 = TesEBinormal[0];
	vec3 b1 = TesEBinormal[1];

	float u = gl_TessCoord.x;
	float v = gl_TessCoord.y;

	vec3 center = (1-u)*p0 + u*p1;
	float radius = (1-u)*Radius[0] + u*Radius[1];

	vec3 n = slerp(n0, n1, u);	//normalize((1-u)*n0 + u*n1);
	vec3 b = slerp(b0, b1, u);	//normalize((1-u)*b0 + u*b1);

	vec3 normal = cos(v*2.f*M_PI)*b + sin(v*2.f*M_PI)*n;
	vec3 position = center + normal*radius;

	WorldPosition = (model_matrix*vec4(position, 1)).xyz;
	WorldNormal = (model_matrix*vec4(normal, 0)).xyz;

	gl_Position = view_projection_matrix*vec4(WorldPosition, 1);
	*/
}