#include "ColorShader.h"
#include "ColorMat.h"

using namespace glm;

namespace renderlib {

enum {
	VP_MATRIX_LOCATION = 0,
	M_MATRIX_LOCATION,
	VIEW_LOCATION,
	COUNT
};

static vector<pair<GLenum, string>> shaders{
	{ GL_VERTEX_SHADER, "shaders/colorShader.vert" },
	{ GL_FRAGMENT_SHADER, "shaders/colorShader.frag" }
};

ColorShader::ColorShader(map<GLenum, string> defines) {
	createProgram(defines);
	calculateUniformLocations();
}

bool ColorShader::createProgram(map<GLenum, string> defines) {
	programID = createGLProgram(shaders, defines);

	return programID != 0;
}

void ColorShader::calculateUniformLocations() {
	glUseProgram(programID);

	uniformLocations.resize(COUNT);

	//Other uniforms
	uniformLocations[VP_MATRIX_LOCATION] = glGetUniformLocation(programID,
		"view_projection_matrix");
	uniformLocations[M_MATRIX_LOCATION] = glGetUniformLocation(programID,
		"model_matrix");
	uniformLocations[VIEW_LOCATION] = glGetUniformLocation(programID,
		"viewPosition");
}

void ColorShader::loadUniforms(const mat4& vp_matrix, const mat4& m_matrix, vec3 viewPosition) {
	glUniformMatrix4fv(uniformLocations[VP_MATRIX_LOCATION], 1, false, &vp_matrix[0][0]);
	glUniformMatrix4fv(uniformLocations[M_MATRIX_LOCATION], 1, false, &m_matrix[0][0]);
	glUniform3f(uniformLocations[VIEW_LOCATION], viewPosition.x, viewPosition.y, viewPosition.z);
}


void ColorShader::draw(const Camera &cam, Drawable &obj) {
	glUseProgram(programID);
	loadUniforms(cam.getProjectionMatrix()*cam.getCameraMatrix(), obj.getTransform(), cam.getPosition());

	obj.getGeometry().drawGeometry();
	glUseProgram(0);
}

}
