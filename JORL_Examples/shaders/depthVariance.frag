#version 410
//r component contains depth, g contains depth squared
layout (location=0) out vec2 FragmentDepth;

in vec3 FragmentModelViewPosition;

void main(void)
{
    // write colour output without modification
    float mvDepth = FragmentModelViewPosition.z;
    FragmentDepth = vec2(mvDepth, mvDepth*mvDepth);
}

