#version 410
#define M_PI 3.1415926535897932384626433832795

// first output is mapped to the framebuffer's colour index by default
out vec4 PixelColour;

in vec2 FragmentTexCoord;

uniform vec3 camera_position;

uniform sampler2D positionTex;
uniform sampler2D normalTex;

uniform vec3 lightPos;

/*float torranceSparrowLighting(vec3 normal, vec3 position, vec3 viewPosition)
{
	vec3 viewer = normalize(viewPosition - position);
	vec3 light = normalize(lightPos - position);
	vec3 h = normalize(viewer + light);

	return ks*(alpha+2.0)*(0.5/M_PI) * clamp(pow(dot(normal, h), alpha), 0.0, 1.0)
			+ kd*clamp(dot(normal, light), 0.0, 1.0) + ka;
}*/

void main(void)
{
	PixelColour = vec4(normalize(texture(normalTex, FragmentTexCoord).rgb), 1);
}
