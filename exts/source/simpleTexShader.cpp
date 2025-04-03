#include "simpleTexShader.h"
#include "TextureMat.h"

using namespace glm;

namespace renderlib {

enum {
	VP_MATRIX_LOCATION = TextureMat::COUNT,
	M_MATRIX_LOCATION,
	COUNT
};

static vector<pair<GLenum, string>> shaders{
	{ GL_VERTEX_SHADER, "shaders/simpleTex.vert" },
	{ GL_FRAGMENT_SHADER, "shaders/simpleTex.frag" }
};

SimpleTexShader::SimpleTexShader(map<GLenum, string> defines) {
	createProgram(defines);
	calculateUniformLocations();
}

bool SimpleTexShader::createProgram(map<GLenum, string> defines) {
	programID = createGLProgram(shaders, defines);

	return programID;
}

void SimpleTexShader::calculateUniformLocations() {
	glUseProgram(programID);

	//Material uniforms
	uniformLocations.resize(COUNT);
	uniformLocations[TextureMat::TEXTURE_LOCATION] = 
		glGetUniformLocation(programID, "colorTexture");

	//Other uniforms
	uniformLocations[VP_MATRIX_LOCATION] = glGetUniformLocation(programID,
		"view_projection_matrix");
	uniformLocations[M_MATRIX_LOCATION] = glGetUniformLocation(programID,
		"model_matrix");
}

void SimpleTexShader::loadUniforms(const mat4& vp_matrix, const mat4& m_matrix) {
	glUniformMatrix4fv(uniformLocations[VP_MATRIX_LOCATION], 1, false, &vp_matrix[0][0]);
	glUniformMatrix4fv(uniformLocations[M_MATRIX_LOCATION], 1, false, &m_matrix[0][0]);
}

void SimpleTexShader::draw(const Camera &cam, Drawable &obj) {
	glUseProgram(programID);
	loadUniforms(cam.getProjectionMatrix()*cam.getCameraMatrix(), obj.getTransform());
	obj.loadUniforms(TextureMat::id, &uniformLocations[0]);

	obj.getGeometry().drawGeometry(programID);
	glUseProgram(0);
}

}