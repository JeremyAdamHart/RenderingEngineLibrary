#include "ColorShader.h"
#include "ColorSetMat.h"
#include "ShadedMat.h"

using namespace glm;
using namespace std;

namespace renderlib {

enum {
	VP_MATRIX_LOCATION = ShadedMat::COUNT + ColorSetMat::COUNT,
	M_MATRIX_LOCATION,
	VIEW_LOCATION,
	LIGHT_POS_LOCATION,
	COUNT
};

static vector<pair<GLenum, string>> shaders{
	{ GL_VERTEX_SHADER, "shaders/colorShader.vert" },
	{ GL_FRAGMENT_SHADER, "shaders/colorShader.frag" }
};

ColorShader::ColorShader(int maxColorNum) {
	createProgram({
		{GL_VERTEX_SHADER, 
		string("#define MAX_COLOR_NUM " + to_string(maxColorNum) + "\n") }});
	calculateUniformLocations();
}

ColorShader::ColorShader(map<GLenum, string> defines) {
	createProgram(defines);
	calculateUniformLocations();
}

bool ColorShader::createProgram(map<GLenum, string> defines) {
	programID = createGLProgram(shaders, defines);

	return programID;
}

bool ColorShader::createNewProgram(vector<pair<GLenum, string>> shaderNames, map<GLenum, string> defines) {
	programID = createGLProgram(shaderNames, defines);

	if (programID)
		calculateUniformLocations();

	return programID;
}

void ColorShader::calculateUniformLocations() {
	glUseProgram(programID);

	uniformLocations.resize(COUNT);
	uniformLocations[ShadedMat::KA_LOCATION] = 
		glGetUniformLocation(programID, "ka");
	uniformLocations[ShadedMat::KS_LOCATION] = 
		glGetUniformLocation(programID, "ks");
	uniformLocations[ShadedMat::KD_LOCATION] = 
		glGetUniformLocation(programID, "kd");
	uniformLocations[ShadedMat::ALPHA_LOCATION] = 
		glGetUniformLocation(programID, "alpha");
	uniformLocations[ShadedMat::COUNT + ColorSetMat::COLORS_LOCATION] =
		glGetUniformLocation(programID, "colors");
	//Other uniforms
	uniformLocations[VP_MATRIX_LOCATION] = 
		glGetUniformLocation(programID, "view_projection_matrix");
	uniformLocations[M_MATRIX_LOCATION] = 
		glGetUniformLocation(programID, "model_matrix");
	uniformLocations[VIEW_LOCATION] = 
		glGetUniformLocation(programID, "viewPosition");
	uniformLocations[LIGHT_POS_LOCATION] =
		glGetUniformLocation(programID, "lightPos");
}

void ColorShader::loadUniforms(const mat4& vp_matrix, const mat4& m_matrix, vec3 viewPosition, vec3 lightPos) {
	glUniformMatrix4fv(uniformLocations[VP_MATRIX_LOCATION], 1, false, &vp_matrix[0][0]);
	glUniformMatrix4fv(uniformLocations[M_MATRIX_LOCATION], 1, false, &m_matrix[0][0]);
	glUniform3f(uniformLocations[VIEW_LOCATION], viewPosition.x, viewPosition.y, viewPosition.z);
	glUniform3f(uniformLocations[LIGHT_POS_LOCATION], lightPos.x, lightPos.y, lightPos.z);
}


void ColorShader::draw(const Camera &cam, glm::vec3 lightPos, Drawable &obj) {
	glUseProgram(programID);
	loadUniforms(cam.getProjectionMatrix()*cam.getCameraMatrix(), obj.getTransform(), cam.getPosition(), lightPos);
	obj.loadUniforms(ShadedMat::id, &uniformLocations[0]);
	obj.loadUniforms(ColorSetMat::id, &uniformLocations[ShadedMat::COUNT]);
	obj.getGeometry().drawGeometry(programID);
	glUseProgram(0);
}

VertexColorShader::VertexColorShader() : ShaderT<>(
	{{ GL_VERTEX_SHADER, "shaders/vertexColor.vert" }, { GL_FRAGMENT_SHADER, "shaders/vertexCOlor.frag" }}, {},
	{ "model_view_projection_matrix" })
{}

void VertexColorShader::draw(Camera & cam, Drawable & obj)
{
	if (!programID) {
		printf("VertexColorShader: Shader compilation failure\n");
		return;
	}
	glUseProgram(programID);

	loadMaterialUniforms(obj);
	glm::mat4 modelViewProjectionMatrix = cam.getProjectionMatrix()*cam.getCameraMatrix()*obj.getTransform();
	glUniformMatrix4fv(uniformLocations[0], 1, false, &modelViewProjectionMatrix[0][0]);
	
	obj.getGeometry().drawGeometry(programID);

	glUseProgram(0);
}



}
