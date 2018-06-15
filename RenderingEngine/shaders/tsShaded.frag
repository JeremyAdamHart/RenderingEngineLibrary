#version 410
#define M_PI 3.1415926535897932384626433832795

// first output is mapped to the framebuffer's colour index by default
out vec4 PixelColour;

in vec3 ModelPosition;
in vec3 FragmentNormal;
in vec2 FragmentTexCoord;

uniform vec3 camera_position;

#ifdef USING_TEXTURE
	uniform sampler2D colorTexture;
#else
	uniform vec4 color;
#endif


uniform vec3 lightPos;
uniform float ks = 0.5;
uniform float kd = 0.4;
uniform float alpha = 5.0;
uniform float ka = 0.3;

float torranceSparrowLighting(vec3 normal, vec3 position, vec3 viewPosition)
{
	vec3 viewer = normalize(viewPosition - position);
	vec3 light = normalize(lightPos - position);
	vec3 h = normalize(viewer + light);

	return ks*(alpha+2.0)*(0.5/M_PI) * clamp(pow(dot(normal, h), alpha), 0.0, 1.0)
			+ kd*clamp(dot(normal, light), 0.0, 1.0) + ka;
}

void main(void)
{
	#ifdef USING_TEXTURE
		vec3 baseColor = texture(colorTexture, vec2(FragmentTexCoord.x, FragmentTexCoord.y)).rgb;
		float alpha = 1.0;
	//	baseColor = vec3(0.0, 1-FragmentTexCoord.y, 0.0);
	#else
		vec3 baseColor = color.xyz;
		float alpha = color.a;
	#endif

 	vec3 shadedColor = torranceSparrowLighting(normalize(FragmentNormal), ModelPosition, camera_position)
 	*baseColor;

 	PixelColour = vec4(shadedColor, alpha);
}
