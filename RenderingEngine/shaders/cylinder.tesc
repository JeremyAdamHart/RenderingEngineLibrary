#version 450

layout(vertices=2) out;

in vec3 TesCNormal [];

out vec3 TesENormal [];
out vec3 TesEBinormal [];
out float Radius [];

void main(){

	vec3 p0 = gl_in[0].gl_Position;
	vec3 p1 = gl_in[1].gl_Position;

	gl_out[gl_InvocationID] = gl_in[gl_InvocationID];

	if(gl_InvocationID == 0){
		
		vec3 n0 = TesCNormal[0];
		vec3 n1 = TesCNormal[1];

		TesENormal[0] = normalize(n0);
		TesENormal[1] = normalize(n1);
		Radius[0] = length(n0);
		Radius[1] = length(n1);

		TesEBinormal[0] = normalize(cross(p1 - p0, n0));
		TesEBinormal[1] = normalize(cross(p1 - p0, n1));
	}


}