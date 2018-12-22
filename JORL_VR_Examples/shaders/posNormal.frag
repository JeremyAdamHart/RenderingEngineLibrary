#version 410

// first output is mapped to the framebuffer's colour index by default
layout (location=0) out vec3 FragmentPosition;
#ifndef POSITION_ONLY
	layout (location=1) out vec3 FragmentNormal;
#endif

in vec4 ModelPosition;
#ifndef POSITION_ONLY
	in vec3 ModelNormal;
#endif

void main(void)
{
    // write colour output without modification
    FragmentPosition = ModelPosition.xyz;
    #ifndef POSITION_ONLY
   		FragmentNormal = ModelNormal;
    #endif
}
