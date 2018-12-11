#version 410

// location indices for these attributes correspond to those specified in the
// InitializeGeometry() function of the main program
layout(location = 0) in vec3 VertexPosition;
#ifndef POSITION_ONLY
	layout(location=1) in vec3 VertexNormal;
#endif

out vec4 ModelPosition;
#ifndef POSITION_ONLY
	out vec3 ModelNormal;
#endif

uniform mat4 model_matrix;
uniform mat4 view_projection_matrix;

void main()
{
    // assign vertex position without modification
    ModelPosition = model_matrix*vec4(VertexPosition, 1.0);
    #ifndef POSITION_ONLY
	    vec4 normal = model_matrix*vec4(VertexNormal, 0.0);
	    ModelNormal = normal.xyz;
    #endif
    gl_Position = view_projection_matrix*ModelPosition;
}
