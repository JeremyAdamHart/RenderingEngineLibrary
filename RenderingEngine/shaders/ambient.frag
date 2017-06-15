#version 410
#define M_PI 3.1415926535897932384626433832795

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
const int NUM_DIRECTIONS = 16;
const float ROT_ANGLE = (2.0*M_PI)/float(NUM_DIRECTIONS);
const float THRESHOLD = 0.1;

const float ks = 0.0;
const float kd = 0.0;
const float ka = 0.9;
const float alpha = 10.0;

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

	vec3 h = normalize(viewer + light);

	return ks*(alpha+2.0)*(0.5/M_PI) * pow(clamp(dot(normal, h), 0.0, 1.0), alpha)
			+ kd*clamp(dot(normal, light), 0.0, 1.0);
}

void main(void)
{
	//PixelColour = vec4(normalize(texture(normalTex, FragmentTexCoord).rgb), 1);
	float ambientOcclusion = calculateAmbientOcclusion();
	vec3 position = texture(positionTex, FragmentTexCoord).rgb;

	if(length(position - vec3(0, 0, 0)) < 0.00001)
		PixelColour = vec4(0, 0, 0, 1);
	else{
		vec3 normal = normalize(texture(normalTex, FragmentTexCoord).rgb);
		float lighting = torranceSparrowLighting(normal, position, camera_position);
		PixelColour = vec4(vec3(1, 1, 1)*((1-ambientOcclusion)*ka + lighting), 1);
	}

}
