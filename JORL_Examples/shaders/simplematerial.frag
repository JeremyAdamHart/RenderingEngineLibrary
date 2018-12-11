#version 410

// first output is mapped to the framebuffer's colour index by default
out vec4 FragmentColour;

void main(void)
{
    // write colour output without modification
    FragmentColour = vec4(1, 0, 0, 1);
}
