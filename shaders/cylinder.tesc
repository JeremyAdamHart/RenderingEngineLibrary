#version 450

layout(vertices=2) out;

in vec3 TesCNormal [];

out vec3 TesENormal [];
out vec3 TesEBinormal [];
out vec3 TesENormal_Par [];
out float Radius [];
//out float Angle;

//Dir is "z-axis" direction for direction of angle
float angleBetween(vec3 a, vec3 b, vec3 dir){
	float cosTheta = dot(a, b);
	float sign = sign(dot(cross(a, b), dir));
	return acos(cosTheta)*sign;
}

void main(){

	vec3 p0 = gl_in[0].gl_Position.xyz;
	vec3 p1 = gl_in[1].gl_Position.xyz;

	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;


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

	vec3 p0p1 = normalize(p1 - p0);
	vec3 n = TesCNormal[gl_InvocationID];
	vec3 n_par = dot(n, p0p1)*p0p1;
	vec3 n_perp = n - n_par;

	TesENormal[gl_InvocationID] = normalize(n_perp);
	TesENormal_Par[gl_InvocationID] = n_par/length(n_perp);
	Radius[gl_InvocationID] = length(n);

	TesEBinormal[gl_InvocationID] = normalize(cross(p1 - p0, n));

	if(gl_InvocationID == 0){
		
		/*vec3 n2 = TesCNormal[1];
		vec3 n_par2 = dot(n2, p0p1)/dot(n2,n2)*n_perp2;
		vec3 n_perp2 = n2 - n_par2;

		float angle = */

		gl_TessLevelOuter[0] = gl_TessLevelOuter[2] = 20;
		gl_TessLevelOuter[1] = gl_TessLevelOuter[3] = 20;
		gl_TessLevelInner[0] = gl_TessLevelInner[1] = 20;


	}
}