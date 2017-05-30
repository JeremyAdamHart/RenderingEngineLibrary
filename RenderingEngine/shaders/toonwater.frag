#version 410
#define M_PI 3.1415926535897932384626433832795

// first output is mapped to the framebuffer's colour index by default
out vec4 FragmentColour;

in vec3 TessNormal;
in vec3 ModelPosition;


uniform vec3 camera_position;

uniform float elapsedTime;

struct WaveFunction
{
	vec2 dir;
	vec2 origin;
	float wavelength;
	float speed;
	float height;
};

uniform WaveFunction waves [MAX_WAVE_NUMBER];

vec3 lightPos = vec3(1000.0, 1000.0, 1000.0);
float ks = 0.5;
float kd = 0.4;
float alpha = 5.0;
float ka = 0.6;
const vec3 COLOR = vec3(0.1, 0.7, 1.0);

const float WIDTH = 0.4;

const float CONTOUR_START = 0.08;
const float CONTOUR_END = 0.02;

vec3 df(WaveFunction w, vec2 pos, float t) {
	float falloff = 1.0;	//clamp((50.0*w.speed - distance(pos, w.origin))/(w.speed*50.0), 0, 1);
	float cosValue = falloff*cos(2.f*M_PI*dot(w.dir, (pos - w.origin)) / (w.wavelength) + w.speed*t*2.f*M_PI);


	vec3 df_dx = vec3(
		1, 
		(w.height*2.0*M_PI/w.wavelength)*w.dir.x*cosValue,
		0);

	vec3 df_dz = vec3(
		0,
		(w.height*2.0*M_PI/w.wavelength)*w.dir.y*cosValue,
		1);


	return normalize(cross(df_dz, df_dx));
}

vec2 df2(WaveFunction w, vec2 pos, float t) {
	float cosValue = cos(2.f*M_PI*dot(w.dir, (pos - w.origin)) / (w.wavelength) + w.speed*t*2.f*M_PI);


	float df_dx =  (w.height*2.0*M_PI/w.wavelength)*w.dir.x*cosValue;

	float df_dz = (w.height*2.0*M_PI/w.wavelength)*w.dir.y*cosValue;


	return vec2(df_dx, df_dz);
}

float torranceSparrowLighting(vec3 normal, vec3 position, vec3 viewPosition)
{
	vec3 viewer = normalize(viewPosition - position);
	vec3 light = normalize(lightPos);
	vec3 h = normalize(viewer + light);

	return ks*(alpha+2.0)*(0.5/M_PI) * clamp(pow(dot(normal, h), alpha), 0.0, 1.0)
			+ kd*clamp(dot(normal, light), 0.0, 1.0) + ka;
}

void main(void)
{
    // write colour output without modification
 //   FragmentColour = vec4(TessNormal, 1.0);

 	vec3 normal = vec3(0, 0, 0);

	/*for(int i=0; i<MAX_WAVE_NUMBER; i++){
		normal += df(waves[i], vec2(ModelPosition.x, ModelPosition.z), elapsedTime);
	}*/

	vec2 df = vec2(0, 0);
	for(int i=0; i<MAX_WAVE_NUMBER; i++){
		df += df2(waves[i], vec2(ModelPosition.x, ModelPosition.z), elapsedTime);
	}

	normal = cross(vec3(0, df.y, 1), vec3(1, df.x, 0));

	normal = normalize(normal);

 	vec3 color = torranceSparrowLighting(normal, ModelPosition, camera_position)*COLOR;
// 	color = normal;	//0.5*(normal + vec3(1, 1, 1));

 	//Draw grid lines
 	if(((ModelPosition.x/WIDTH) - floor(ModelPosition.x/WIDTH) < WIDTH/8.f) ||
 		((ModelPosition.z/WIDTH) - floor(ModelPosition.z/WIDTH) < WIDTH/8.f))
 		color = vec3(0.1, 0.1, 0.1);
 	vec3 projectedView = camera_position - ModelPosition;
// 	projectedView.y = 0;
 	float dotNormalView = max(dot(normalize(normal), normalize(projectedView)), 0);

 	float contourStart = 50.0*CONTOUR_START/pow(distance(camera_position, ModelPosition), 1);
	float contourEnd = 50.0*CONTOUR_END/pow(distance(camera_position, ModelPosition), 1);

 	if(dotNormalView < contourStart){
 		const vec3 CONTOUR_COLOR = vec3(0.9, 0.9, 0.9);

		float u =max((dotNormalView - contourEnd)/(contourStart - contourEnd), 0);
		color = (1-u)*CONTOUR_COLOR + u*color;
 	}

 	FragmentColour = vec4(color, 1);
}
