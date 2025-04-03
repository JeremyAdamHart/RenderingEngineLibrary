#version 410

// first output is mapped to the framebuffer's colour index by default
out vec4 FragmentColour;

#ifdef NUM_SAMPLES
sampler2DMS opaqueTexture;
sampler2DMS translucentTexture;
#else
sampler2D opaqueTexture;
sampler2D translucentTexture;
#endif

ivec3 FragmentCoord;

void main()
{
	vec4 opaqueColour = vec4(0.0);
	vec4 translucentColour = vec4(0.0);
	#ifdef NUM_SAMPLES
	for(int i=0; i<NUM_SAMPLES; i++){
		opaqueColour += texelFetch(opaqueTexture, FragmentCoord, i);
	}
	opaqueColour /= NUM_SAMPLES;
	for(int i=0; i<NUM_SAMPLES; i++){
		translucentColour += texelFetch(translucentTexture, FragmentCoord, i);
	}
	translucentColour /= NUM_SAMPLES;
	#else
	opaqueColour = texelFetch(opaqueTexture, FragmentCoord);
	translucentColour = texelFetch(translucentTexture, FragmentCoord);
	#endif

	float alpha = min(translucentColour.a, 1);
	FragmentColour = (1.0-alpha)*opaqueColour + alpha*translucentColour; 
}
