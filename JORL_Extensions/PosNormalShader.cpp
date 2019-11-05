#include "PosNormalShader.h"

#include <algorithm>

namespace renderlib {

using namespace glm;

enum {
	VP_MATRIX_LOCATION = 0,
	M_MATRIX_LOCATION,
	COUNT
};

static vector<pair<GLenum, string>> shaders{
	{ GL_VERTEX_SHADER, "shaders/posNormal.vert" },
	{ GL_FRAGMENT_SHADER, "shaders/posNormal.frag" }
};

PosNormalShader::PosNormalShader(map<GLenum, string> defines)
{
	createProgram(defines);
	calculateUniformLocations();
}

PosNormalShader::PosNormalShader(PNShaderOutput output)
{
	if (output == PNShaderOutput::POSITION)
		createProgram({ {GL_FRAGMENT_SHADER, "#define POSITION_ONLY\n"} });
	else
		createProgram();
	calculateUniformLocations();
}

bool PosNormalShader::createProgram(map<GLenum, string> defines) {

	programID = createGLProgram(shaders, defines);

	return programID;
}

void PosNormalShader::calculateUniformLocations() {
	glUseProgram(programID);

	uniformLocations.resize(COUNT);

	uniformLocations[VP_MATRIX_LOCATION] = glGetUniformLocation(programID,
		"view_projection_matrix");
	uniformLocations[M_MATRIX_LOCATION] = glGetUniformLocation(programID,
		"model_matrix");
}

void PosNormalShader::loadUniforms(const mat4& vp_matrix, 
	const mat4& m_matrix) {
	glUniformMatrix4fv(uniformLocations[VP_MATRIX_LOCATION], 1, false, &vp_matrix[0][0]);
	glUniformMatrix4fv(uniformLocations[M_MATRIX_LOCATION], 1, false, &m_matrix[0][0]);
}

void PosNormalShader::draw(const Camera &cam, vec3 lightPos, 
	Drawable &obj) {
	glUseProgram(programID);
	loadUniforms(cam.getProjectionMatrix()*cam.getCameraMatrix(), 
		obj.getTransform());
	
	obj.getGeometry().drawGeometry(programID);
	glUseProgram(0);
}

}