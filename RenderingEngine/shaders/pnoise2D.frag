#version 410
#define M_PI 3.1415926535897932384626433832795
#define MOD_MAX 8388608

// first output is mapped to the framebuffer's colour index by default
out vec4 PixelColour;

in vec2 FragmentTexCoord;

//Perlin noise values
uniform int octaveNum = 1;
uniform float baseWidth = 0.1;
uniform float persistance = 0.5;

uint hash( uint x ) {
    x += ( x << 10u );
    x ^= ( x >>  6u );
    x += ( x <<  3u );
    x ^= ( x >> 11u );
    x += ( x << 15u );
    return x;
}

//Linear congruential generator - Doesn't work well for this
uint lcg(uint seed){
	return (65793*seed+4282663)%MOD_MAX;
}

vec2 gradient(uint i, uint j, uint seed){
	uint rand = hash(hash(seed+i)+j)%MOD_MAX;
	float theta = 2.0*M_PI*float(rand)/float(MOD_MAX);
	return vec2(sin(theta), cos(theta));
}

float fade(float t){
	return t*t*t*(t*(t*6 - 15) + 10);
}

float generateNoise(float x, float y, float width, uint seed){
	uint i = uint(x/width);
	uint j = uint (y/width);
	float x_r = mod(x, width)/width;
	float y_r = mod(y, width)/width;

	vec2 g00 = gradient(i, j, seed);
	vec2 g01 = gradient(i+1, j, seed);
	vec2 g10 = gradient(i, j+1, seed);
	vec2 g11 = gradient(i+1, j+1, seed);

	float v00 = dot(g00, vec2(0, 0) - vec2(x_r, y_r));
	float v01 = dot(g01, vec2(1, 0) - vec2(x_r, y_r));
	float v10 = dot(g10, vec2(0, 1) - vec2(x_r, y_r));
	float v11 = dot(g11, vec2(1, 1) - vec2(x_r, y_r));

	float s = fade(x_r);
	float t = fade(y_r);

	//Bilinear interpolation
	float v0 = (1-s)*v00 + s*v01;
	float v1 = (1-s)*v10 + s*v11;
	return (1-t)*v0 + t*v1;
}

void main(void)
{
	float noise = 0.0;
	for(int i=0; i<5; i++){
		float weight = pow(persistance, i+1);
		float width = baseWidth*pow(0.5, i);
		noise += (generateNoise(FragmentTexCoord.x, FragmentTexCoord.y, width, 19831)+0.5)*weight;
	}

	float turbulence = 0.5;
	float intensity = sin((FragmentTexCoord.x + FragmentTexCoord.y+noise*turbulence)*20.0)*0.5 + 0.5;

	PixelColour = vec4(intensity*vec3(1, 1, 1), 1);
}
