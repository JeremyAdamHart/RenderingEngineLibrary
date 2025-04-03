#version 410

// location indices for these attributes correspond to those specified in the
// InitializeGeometry() function of the main program
layout(location = 0) in vec3 VertexPosition;
layout(location = 1) in vec3 ParticlePosition;
layout(location = 2) in vec3 ParticleVelocity;
layout(location = 3) in float ParticleHeat;

uniform mat4 view_projection_matrix;
uniform mat4 inv_camera_rotation_matrix;
uniform float radius;

out vec3 particleOrigin;
out vec3 positionWorldSpace;
out vec3 velocity;
out float heat;

void main()
{
   	//Translate to origin and undo camera rotation
    mat4 translateToOrigin;
    translateToOrigin[3] = vec4(-ParticlePosition, 1);
    mat4 translateToParticle=mat4(1.f);
    translateToParticle[3] = vec4(ParticlePosition, 1);
    mat4 scaleMatrix = mat4(1.f);
    scaleMatrix *= radius;
    scaleMatrix[3][3] = 1.0;

    mat4 model_matrix = translateToParticle*inv_camera_rotation_matrix*scaleMatrix;
//	mat4 model_matrix = translateToParticle*scaleMatrix;

    particleOrigin = ParticlePosition;
    positionWorldSpace = (model_matrix*vec4(VertexPosition, 1.0)).xyz;
    velocity = ParticleVelocity;
    heat = ParticleHeat;

    gl_Position = view_projection_matrix*model_matrix*vec4(VertexPosition, 1.0);

}
