#version 410

// first output is mapped to the framebuffer's colour index by default
out vec4 FragmentColour;

uniform sampler2D colorTexture;

in vec2 FragmentTexCoord;

void main(void)
{
    // write colour output without modification
    FragmentColour = texture(colorTexture, FragmentTexCoord);
}
