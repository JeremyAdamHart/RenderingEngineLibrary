#version 410
#define M_PI 3.1415926535897932384626433832795
#define MOD_MAX 8388608

// first output is mapped to the framebuffer's colour index by default
out vec4 PixelColour;

in vec2 FragmentTexCoord;

uniform vec3 camera_position;
uniform vec3 cam_up;
uniform vec3 cam_right;

uniform sampler2D positionTex;
uniform sampler2D normalTex;

uniform vec3 lightPos;

const float MAX_DIST = 0.2;
const int NUM_SAMPLES = 10;
const float SAMPLE_STEP = MAX_DIST/float(NUM_SAMPLES-1);
const int NUM_DIRECTIONS = 8;
const float ROT_ANGLE = (2.0*M_PI)/float(NUM_DIRECTIONS);
const float THRESHOLD = 0.1;

const float ks = 0.2;
const float kd = 0.2;
const float ka = 0.6;
const float alpha = 200.0;

vec3 getTangentOfNormal(vec3 normal, vec3 direction){
	vec3 n_proj_d = dot(normal, direction)/dot(direction, direction)*direction;
	float d_length = length(n_proj_d);
	float n_length = length(normal - n_proj_d);
	vec3 n_comp = (normal - n_proj_d)/n_length;
	vec3 d_comp = n_proj_d/d_length;

	return normalize(d_comp*n_length - n_comp*d_length); 
}

float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

float calculateAmbientOcclusion(vec3 position, vec3 normal, vec2 texDir){
	vec3 direction = normalize(cam_right*texDir.x + cam_up*texDir.y);
//	vec3 tangent = getTangentOfNormal(normal, direction);

	vec2 texPos = FragmentTexCoord;
	float maxOcclusion = 0.0;

	for(int i=0; i<NUM_SAMPLES; i++){
		texPos += texDir*SAMPLE_STEP;
		vec3 samplePos = texture(positionTex, texPos).rgb;

		//If point is not the clear color
		if(length(samplePos - vec3(0, 0, 0)) > 0.00001){
			vec3 sampleDir = normalize(samplePos - position);
			if(dot(sampleDir, normal) > 0.0){
//				float cos_theta = dot(sampleDir, tangent);
				float r = length(samplePos - position)/MAX_DIST;
				float occlusion = max(dot(sampleDir, normal)-THRESHOLD, 0.0)*(1.0-r*r);	//*1.0/length(samplePos - position); 	//1.0 - cos_theta*cos_theta;
				if(occlusion > maxOcclusion)
					maxOcclusion = occlusion;
			}
		}
	}

	return maxOcclusion;
}

float calculateAmbientOcclusion(){
	float totalOcclusion = 0.0;

	vec3 position = texture(positionTex, FragmentTexCoord).rgb;
	vec3 normal = normalize(texture(normalTex, FragmentTexCoord).rgb);

	float angle = M_PI*2.0*rand(position.xy);

	for(int i=0; i<NUM_DIRECTIONS; i++){
		vec2 texDir = vec2(cos(angle), sin(angle));
		totalOcclusion += calculateAmbientOcclusion(position, normal, texDir);
		angle += ROT_ANGLE;
	}

	return totalOcclusion/float(NUM_DIRECTIONS);
}

float torranceSparrowLighting(vec3 normal, vec3 position, vec3 viewPosition)
{
	vec3 viewer = normalize(viewPosition - position);
	vec3 light = normalize(lightPos - position);
	float attenuation = 1.f; //320.f/(length(position - lightPos)*length(position-lightPos));

	vec3 h = normalize(viewer + light);
	//Formula found here: http://www.farbrausch.de/~fg/stuff/phong.pdf
	float normalizationFactor = (alpha+2)*(alpha+4)/(8*M_PI*(pow(sqrt(2), -alpha)+alpha));
//	energyConservation = (alpha+2.0)*(0.5/M_PI);
	
/*	return max(dot(normal, light), 0)*ks*(alpha+2.0)*(0.5/M_PI) * pow(clamp(dot(normal, h), 0.0, 1.0), alpha)
			+ kd*clamp(dot(normal, light), 0.0, 1.0);*/
	return max(dot(normal, light), 0)* (ks*normalizationFactor * pow(clamp(dot(normal, h), 0.0, 1.0), alpha)
			+ kd*clamp(dot(normal, light), 0.0, 1.0))*attenuation;

}

float phongLighting(vec3 normal, vec3 position, vec3 viewPosition)
{
	vec3 viewer = normalize(viewPosition - position);
	vec3 light = normalize(lightPos - position);

	vec3 r = normalize(2.0*dot(light, normal)*normal - light);

	return ks*pow(clamp(dot(normal, r), 0.0, 1.0), alpha)
			+ kd*clamp(dot(normal, light), 0.0, 1.0);
}

////////////////
//PERLIN NOISE
////////////////
//Perlin noise values
uniform int octaveNum = 8;
uniform float baseWidth = 0.1;
uniform float persistance = 0.5;
uniform int seedValue = 71;
uniform float turbulence = 1.0;

uint hash( uint x ) {
    x += ( x << 10u );
    x ^= ( x >>  6u );
    x += ( x <<  3u );
    x ^= ( x >> 11u );
    x += ( x << 15u );
    return x;
}

vec3 gradient(uint i, uint j, uint k, uint seed){
	uint rand = hash(hash(hash(seed+i)+j)+k)%MOD_MAX;
	uint rand2 = hash(rand)%MOD_MAX;
	float theta = 2.0*M_PI*float(rand)/float(MOD_MAX);
	float u = 2.0*float(rand2)/float(MOD_MAX) - 1.0;
	return vec3(sqrt(1 - u*u)*sin(theta), sqrt(1 - u*u)*cos(theta), u);
}

float fade(float t){
	return t*t*t*(t*(t*6 - 15) + 10);
}

float generateNoise(float x, float y, float z, float width, uint seed){
	uint i = uint(x/width);
	uint j = uint (y/width);
	uint k = uint (z/width);
	float x_r = mod(x, width)/width;
	float y_r = mod(y, width)/width;
	float z_r = mod(z, width)/width;

	vec3 g000 = gradient(i, j, k, seed);
	vec3 g010 = gradient(i+1, j, k, seed);
	vec3 g100 = gradient(i, j+1, k, seed);
	vec3 g110 = gradient(i+1, j+1, k, seed);

	vec3 g001 = gradient(i, j, k+1, seed);
	vec3 g011 = gradient(i+1, j, k+1, seed);
	vec3 g101 = gradient(i, j+1, k+1, seed);
	vec3 g111 = gradient(i+1, j+1, k+1, seed);

	float v000 = dot(g000, vec3(0, 0, 0) - vec3(x_r, y_r, z_r));
	float v010 = dot(g010, vec3(1, 0, 0) - vec3(x_r, y_r, z_r));
	float v100 = dot(g100, vec3(0, 1, 0) - vec3(x_r, y_r, z_r));
	float v110 = dot(g110, vec3(1, 1, 0) - vec3(x_r, y_r, z_r));

	float v001 = dot(g001, vec3(0, 0, 1) - vec3(x_r, y_r, z_r));
	float v011 = dot(g011, vec3(1, 0, 1) - vec3(x_r, y_r, z_r));
	float v101 = dot(g101, vec3(0, 1, 1) - vec3(x_r, y_r, z_r));
	float v111 = dot(g111, vec3(1, 1, 1) - vec3(x_r, y_r, z_r));
	

	float s = fade(x_r);
	float t = fade(y_r);
	float r = fade(z_r);

	//Bilinear interpolation
	float v00 = (1-s)*v000 + s*v010;
	float v10 = (1-s)*v100 + s*v110;
	float v01 = (1-s)*v001 + s*v011;
	float v11 = (1-s)*v101 + s*v111;
	float v0 = (1-t)*v00 + t*v10;
	float v1 = (1-t)*v01 + t*v11;
	return (1-r)*v0 + r*v1;
}

float sun(vec2 coord, float noise){
	vec2 input = vec2(coord.x, coord.y);
	return 1.0 - 2.0*(length(input - vec2(0.5, 0.5))+noise);
}

float spots(vec2 coord, float noise, float period){
	return (sin((noise+coord.x)*2.0*M_PI/period)+1.0)*0.5 * 
			(sin((noise+coord.y)*2.0*M_PI/period)+1.0)*0.5;
}

float rings(vec2 coord, float noise, float period){
	float x = length(coord - vec2(0.5, 0.5));
	return cos((noise + x)*2.0*M_PI/period);
}

float lightning(vec2 coord, float noise, float width){
	float x = max(1.0 - abs(coord.x+noise - 0.5)/width, 0.0);
	return x*x*x*x;
}

float sinLines(vec3 coord, float noise, float noiseB){
	float period = 0.25;
	return ( (sin((coord.x + coord.y)*2.0*M_PI/period + noise)*0.5 + 0.5)
		*(sin((coord.z)*2.0*M_PI/period + noiseB)*0.5 + 0.5));
}

float radialSin(vec3 coord, float noise){
	float period = 0.2;
	return sin((length(coord) + noise)*2.0*M_PI/period)*0.5 + 0.5;
}


void main(void)
{
	//PixelColour = vec4(normalize(texture(normalTex, FragmentTexCoord).rgb), 1);
	float ambientOcclusion = calculateAmbientOcclusion();
	vec3 position = texture(positionTex, FragmentTexCoord).rgb;

	vec3 coord = position*0.75 + vec3(0.75, 0.75, 0.75);

	//Perlin noise
	float noise = 0.0;
	float noiseB = 0.0;
	for(int i=0; i<octaveNum; i++){
		float weight = pow(persistance, i+1);
		float width = baseWidth*pow(0.5, i);
		noise += (generateNoise(coord.x, coord.y, coord.z, width, seedValue)+0.5)*weight;
		noiseB += (generateNoise(coord.x, coord.y, coord.z, width, 123931)+0.5)*weight;
	}

//	float intensity = sinLines(coord, noise*turbulence, noiseB*turbulence);
	float intensity = radialSin(coord, noise*turbulence);
//	float intensity = noise;
	vec3 color = (1.0 - intensity*intensity*intensity*intensity*intensity)*vec3(1, 1, 1) + vec3(0.1, 0.07, 0.1);

//	vec3 color = vec3(1, 1, 1);

	if(length(position - vec3(0, 0, 0)) < 0.00001)
		PixelColour = vec4(0, 0, 0, 1);
	else{
		vec3 normal = normalize(texture(normalTex, FragmentTexCoord).rgb);
		float lighting = torranceSparrowLighting(normal, position, camera_position);
		PixelColour = vec4(color*((1-ambientOcclusion)*ka + lighting), 1);
	}

}
