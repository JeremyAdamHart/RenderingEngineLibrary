#version 450

layout(vertices=2) out;

in vec3 TesCNormal [];

out vec3 TesENormal [];
out vec3 TesEBinormal [];
out float Radius [];

void main(){

	vec3 p0 = gl_in[0].gl_Position.xyz;
	vec3 p1 = gl_in[1].gl_Position.xyz;

	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

	if(gl_InvocationID == 0){
		
		gl_TessLevelOuter[0] = gl_TessLevelOuter[2] = 20;
		gl_TessLevelOuter[1] = gl_TessLevelOuter[3] = 20;
		gl_TessLevelInner[0] = gl_TessLevelInner[1] = 20;


	}

	/*
	vec3 n0 = TesCNormal[0];
	vec3 n1 = TesCNormal[1];

	TesENormal[gl_InvocationID] = normalize(n0);
	TesENormal[gl_InvocationID] = normalize(n1);
	Radius[gl_InvocationID] = length(n0);
	Radius[gl_InvocationID] = length(n1);

	TesEBinormal[gl_InvocationID] = normalize(cross(p1 - p0, n0));
	TesEBinormal[gl_InvocationID] = normalize(cross(p1 - p0, n1));
	*/

	vec3 n = TesCNormal[gl_InvocationID];

	TesENormal[gl_InvocationID] = normalize(n);
	Radius[gl_InvocationID] = length(n);

	TesEBinormal[gl_InvocationID] = normalize(cross(p1 - p0, n));
}