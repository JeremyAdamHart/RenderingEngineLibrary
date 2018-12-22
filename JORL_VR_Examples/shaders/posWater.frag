#version 410

// first output is mapped to the framebuffer's colour index by default
layout(location=0) out vec3 FragmentPosition;

in vec3 ModelPosition;

void main(void)
{
 	FragmentPosition = ModelPosition;
// 	FragmentPosition = ModelPosition.y*vec3(1, 1, 1);
}
