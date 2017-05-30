#include "SimpleShader.h"
#include "ColorMat.h"

using namespace glm;

enum {
	VP_MATRIX_LOCATION=ColorMat::COUNT,
	M_MATRIX_LOCATION,
	COUNT
};

static vector<pair<GLenum, string>> shaders{
	{GL_VERTEX_SHADER, "shaders/simple.vert"},
	{GL_FRAGMENT_SHADER, "shaders/simple.frag"}
};

SimpleShader::SimpleShader(map<GLenum, string> defines) {
	createProgram(defines);
	calculateUniformLocations();
}

bool SimpleShader::createProgram(map<GLenum, string> defines) {
	programID = createGLProgram(shaders, defines);

	return programID != 0;
}

void SimpleShader::calculateUniformLocations() {
	glUseProgram(programID);
	
	//Material uniforms
	uniformLocations.resize(COUNT);
	uniformLocations[ColorMat::COLOR_LOCATION] = glGetUniformLocation(programID, "color");

	//Other uniforms
	uniformLocations[VP_MATRIX_LOCATION] = glGetUniformLocation(programID, 
		"view_projection_matrix");
	uniformLocations[M_MATRIX_LOCATION] = glGetUniformLocation(programID,
		"model_matrix");
}

void SimpleShader::loadUniforms(const mat4& vp_matrix, const mat4& m_matrix) {
	glUniformMatrix4fv(uniformLocations[VP_MATRIX_LOCATION], 1, false, &vp_matrix[0][0]);
	glUniformMatrix4fv(uniformLocations[M_MATRIX_LOCATION], 1, false, &m_matrix[0][0]);
}

void SimpleShader::draw(const Camera &cam, const Drawable &obj) {
	glUseProgram(programID);
	loadUniforms(cam.getProjectionMatrix()*cam.getCameraMatrix(), obj.getTransform());
	obj.loadUniforms(ColorMat::id, &uniformLocations[0]);	
	
	obj.getGeometry().drawGeometry();
	glUseProgram(0);
}

