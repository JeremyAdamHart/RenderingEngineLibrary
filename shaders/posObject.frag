#version 410

// first output is mapped to the framebuffer's colour index by default
layout (location=0) out vec3 FragmentPosition;

in vec4 ModelPosition;

void main(void)
{
    // write colour output without modification
    FragmentPosition = ModelPosition.xyz;
    //FragmentPosition = vec3(1, 0, 0);
}
