#version 410
#define M_PI 3.1415926535897932384626433832795
layout(quads) in;

uniform mat4 view_projection_matrix;
uniform float elapsedTime;

out vec3 TessNormal;
out vec3 ModelPosition;

struct WaveFunction
{
	vec2 dir;
	vec2 origin;
	float wavelength;
	float speed;
	float height;
};

uniform WaveFunction waves [MAX_WAVE_NUMBER];

float f(WaveFunction w, vec2 pos, float t){
	float falloff = 1.0;	//clamp(((50.0*w.speed) - distance(pos, w.origin))/(w.speed*50.0), 0, 1);
	return falloff*w.height*sin(2.f*M_PI*dot(w.dir, (pos - w.origin)) / (w.wavelength) + w.speed*t*2.f*M_PI);
}

vec3 df(WaveFunction w, vec2 pos, float t) {
	float falloff = 1.0;	//clamp((50.0*w.speed - distance(pos, w.origin))/(w.speed*50.0), 0, 1);
	float cosValue = falloff*cos(2.f*M_PI*dot(w.dir, (pos - w.origin)) / (w.wavelength) + w.speed*t*2.f*M_PI);
	vec3 df_dx = vec3(
		1, 
		(w.height*2.0*M_PI/w.wavelength)*(pos.x - w.origin.x)*cosValue,
		0);

	vec3 df_dz = vec3(
		0,
		(w.height*2.0*M_PI/w.wavelength)*(pos.y - w.origin.y)*cosValue,
		1);

	return normalize(cross(df_dz, df_dx));
}

void main()
{
	float u = gl_TessCoord.x;
	float v = gl_TessCoord.y;

	vec3 py0 = (1-u)*gl_in[0].gl_Position.xyz + u*gl_in[1].gl_Position.xyz;
	vec3 py1 = (1-u)*gl_in[2].gl_Position.xyz + u*gl_in[3].gl_Position.xyz;

	vec3 position = (1-v)*py0 + v*py1;
	vec3 color = vec3(0, 0, 0);

	for(int i=0; i<MAX_WAVE_NUMBER; i++){
		position += vec3(0, f(waves[i], vec2(position.x, position.z), elapsedTime), 0);
		color += df(waves[i], vec2(position.x, position.z), elapsedTime);
	}

	ModelPosition = position;
	TessNormal = normalize(color);
	gl_Position = view_projection_matrix*vec4(position, 1.0);
}
