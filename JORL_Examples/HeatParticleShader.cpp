#include "HeatParticleShader.h"
#include "HeatParticleMat.h"

using namespace glm;

using namespace renderlib;

using namespace std;

enum {
	VP_MATRIX_LOCATION=HeatParticleMat::COUNT,
	INV_CAMERA_ROT_LOCATION,
	COUNT
};

static vector<pair<GLenum, string>> shaders{
	{ GL_VERTEX_SHADER, "shaders/heatParticle.vert" },
	{ GL_FRAGMENT_SHADER, "shaders/heatParticle.frag" }
};

HeatParticleShader::HeatParticleShader(map<GLenum, string> defines) {
	createProgram(defines);
	calculateUniformLocations();
}

bool HeatParticleShader::createProgram(map<GLenum, string> defines) {
	programID = createGLProgram(shaders, defines);

	return programID;
}

void HeatParticleShader::calculateUniformLocations() {
	glUseProgram(programID);

	//Material uniforms
	uniformLocations.resize(COUNT);
	uniformLocations[HeatParticleMat::RADIUS_LOCATION] = glGetUniformLocation(programID, "radius");

	//Other uniforms
	uniformLocations[VP_MATRIX_LOCATION] = glGetUniformLocation(programID,
		"view_projection_matrix");
	uniformLocations[INV_CAMERA_ROT_LOCATION] = glGetUniformLocation(programID,
		"inv_camera_rotation_matrix");

	checkGLErrors("HeatParticleShader::calculateUniformLocations");
}

void HeatParticleShader::loadUniforms(const mat4& vp_matrix, const mat4& inv_rot_matrix) {
	glUniformMatrix4fv(uniformLocations[VP_MATRIX_LOCATION], 1, false, &vp_matrix[0][0]);
	glUniformMatrix4fv(uniformLocations[INV_CAMERA_ROT_LOCATION], 1, false, &inv_rot_matrix[0][0]);
}

void HeatParticleShader::draw(const Camera &cam, Drawable &obj) {
	glUseProgram(programID);
	loadUniforms(cam.getProjectionMatrix()*cam.getCameraMatrix(), cam.getRotationMatrix());
	obj.loadUniforms(HeatParticleMat::id, &uniformLocations[0]);

	checkGLErrors("HeatParticleShader::draw->loadUniforms");

	obj.getGeometry().drawGeometry(programID);
	glUseProgram(0);

	checkGLErrors("HeatParticleShader::draw");
}
