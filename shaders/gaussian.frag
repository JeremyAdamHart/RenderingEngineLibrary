#version 410

out vec4 OutputColor;

uniform float sigma;
uniform int n;		//Should be odd
uniform int blurDirection;		//0 for horizontal, 1 for vertical
uniform sampler2D colorTex;

#define M_PI 3.1415926535897932384626433832795

in vec2 FragmentTexCoord;

float gaussian(float x){
	return exp(-x*x/(2.0*sigma*sigma))/(sqrt(2.0*M_PI)*sigma);
}

vec4 applyGaussian(){
	ivec2 dimensions = textureSize(colorTex, 0);
	vec2 step = vec2(
		1.0/float(dimensions.x-1), 
		1.0/float(dimensions.y-1));

	int start = -n/2;
	ivec2 direction = ivec2(1, 1);
	if(blurDirection == 0)
		direction.x = 0;
	else
		direction.y = 0;

	float totalWeight = 0.0;
	vec4 totalColor = vec4(0.0);

	for(int i=0; i<n; i++){
		ivec2 offset = (start+i)*direction;
		vec2 sampleCoord = vec2(offset)*step + FragmentTexCoord;
		float gaussianResult = gaussian(float(start+i));
		totalWeight += gaussianResult;
		//totalColor += gaussianResult*textureOffset(colorTex, FragmentTexCoord, offset);
		totalColor += gaussianResult*texture(colorTex, sampleCoord);
		//return textureOffset(colorTex, FragmentTexCoord, offset);
	}

	return totalColor/totalWeight;
}

void main()
{
	//ivec2 dimension = textureSize(colorTex);	//Consider changing
	//ivec2 pixelTexCoord = ivec2(FragmentTexCoord.x*dimension, TexCoord.y*dimension);
	OutputColor = applyGaussian();
	//OutputColor = texture(colorTex, FragmentTexCoord);
}