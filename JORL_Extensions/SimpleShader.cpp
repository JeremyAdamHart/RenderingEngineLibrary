#include "SimpleShader.h"
#include "ColorMat.h"

using namespace glm;

namespace renderlib {

enum {
	VP_MATRIX_LOCATION=ColorMat::COUNT,
	M_MATRIX_LOCATION,
	COUNT
};

vector<pair<GLenum, string>> SimpleShader::defaultShaders(){
	return{ {GL_VERTEX_SHADER, "shaders/simple.vert"},
	{GL_FRAGMENT_SHADER, "shaders/simple.frag"} };
}

SimpleShader::SimpleShader(map<GLenum, string> defines):Shader(ColorMat::id) {
	createProgram(defines);
	calculateUniformLocations();
}

//Alt shader should use the same uniforms
SimpleShader::SimpleShader(vector<pair<GLenum, string>> alt_shaders, map<GLenum, string> defines) {
	programID = createGLProgram(alt_shaders, defines);
	calculateUniformLocations();
}

bool SimpleShader::createProgram(map<GLenum, string> defines) {
	programID = createGLProgram(defaultShaders(), defines);

	return programID;
}

vector<string> SimpleShader::getUniformNames() { return{ "color", "view_projection_matrix", "model_matrix" }; }

void SimpleShader::loadUniforms(const mat4& vp_matrix, const mat4& m_matrix) {
	glUniformMatrix4fv(uniformLocations[VP_MATRIX_LOCATION], 1, false, &vp_matrix[0][0]);
	glUniformMatrix4fv(uniformLocations[M_MATRIX_LOCATION], 1, false, &m_matrix[0][0]);
}

void SimpleShader::draw(const Camera &cam, Drawable &obj) {
	glUseProgram(programID);
	loadUniforms(cam.getProjectionMatrix()*cam.getCameraMatrix(), obj.getTransform());
	
	loadMaterialUniforms(obj);
	
	obj.getGeometry().drawGeometry(programID);
	glUseProgram(0);
}

}
