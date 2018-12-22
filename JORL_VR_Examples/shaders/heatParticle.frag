#version 410

// first output is mapped to the framebuffer's colour index by default
out vec4 FragmentColour;

uniform float radius;

in vec3 particleOrigin;
in vec3 positionWorldSpace;
in vec3 velocity;
in float heat;

//vec3 particleColor = vec3(1.0, 0.5, 0.2);
vec3 particleColor = vec3(0.2, 0.4, 1.0);

void main()
{
	float linearFalloff = 1 - min(distance(positionWorldSpace, particleOrigin)/radius, 1);
    FragmentColour = vec4(particleColor, heat*linearFalloff*1.0);
}
