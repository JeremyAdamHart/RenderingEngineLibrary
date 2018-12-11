#version 410

// first output is mapped to the framebuffer's colour index by default
out vec4 OutputColor;

in vec3 FragmentColor;
in vec3 WorldNormal;
in vec3 WorldPosition;

uniform vec3 viewPosition;

uniform vec3 lightPos = vec3(-100, 100, 100);

uniform float alpha = 20.0;
uniform float ks = 0.4;
uniform float kd = 0.4;
uniform float ka = 0.4;
#define M_PI 3.1415926535897932384626433832795

float blinnPhongLighting(vec3 normal, vec3 position, vec3 viewPosition)
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
			+ kd*clamp(dot(normal, light), 0.0, 1.0))*attenuation + ka;

}

void main(void)
{
	vec3 color = FragmentColor;

	color = color*blinnPhongLighting(normalize(WorldNormal), WorldPosition, viewPosition);

    // write colour output without modification
    OutputColor = vec4(color, 1.0);
}
