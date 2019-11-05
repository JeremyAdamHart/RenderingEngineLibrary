#include "VRColorShader.h"
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
	FOG_DISTANCE_LOCATION,
	FOG_COLOR_LOCATION,
	FOG_SCALE_LOCATION,
	COUNT
};

static vector<pair<GLenum, string>> shaders{
	{ GL_VERTEX_SHADER, "shaders/marchingCubeColor.vert" },
	{ GL_FRAGMENT_SHADER, "shaders/marchingCubeColor.frag" }
};

VRColorShader::VRColorShader(int maxColorNum) {
	createProgram({
		{GL_VERTEX_SHADER, 
		string("#define MAX_COLOR_NUM " + to_string(maxColorNum) + "\n") }});
	calculateUniformLocations();
}

VRColorShader::VRColorShader(map<GLenum, string> defines) {
	createProgram(defines);
	calculateUniformLocations();
}

bool VRColorShader::createProgram(map<GLenum, string> defines) {
	programID = createGLProgram(shaders, defines);

	return programID;
}

bool VRColorShader::createNewProgram(vector<pair<GLenum, string>> shaderNames, map<GLenum, string> defines) {
	programID = createGLProgram(shaderNames, defines);

	if (programID)
		calculateUniformLocations();

	return programID;
}

void VRColorShader::calculateUniformLocations() {
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
	uniformLocations[FOG_COLOR_LOCATION] =
		glGetUniformLocation(programID, "fogColor");
	uniformLocations[FOG_DISTANCE_LOCATION] =
		glGetUniformLocation(programID, "fogDist");
	uniformLocations[FOG_SCALE_LOCATION] =
		glGetUniformLocation(programID, "fogScale");
}

void VRColorShader::loadUniforms(const mat4& vp_matrix, const mat4& m_matrix, vec3 viewPosition, vec3 lightPos, 
	float fogScale, float fogDistance, glm::vec3 fogColor) 
{
	glUniformMatrix4fv(uniformLocations[VP_MATRIX_LOCATION], 1, false, &vp_matrix[0][0]);
	glUniformMatrix4fv(uniformLocations[M_MATRIX_LOCATION], 1, false, &m_matrix[0][0]);
	glUniform3f(uniformLocations[VIEW_LOCATION], viewPosition.x, viewPosition.y, viewPosition.z);
	glUniform3f(uniformLocations[LIGHT_POS_LOCATION], lightPos.x, lightPos.y, lightPos.z);
	glUniform1f(uniformLocations[FOG_SCALE_LOCATION], fogScale);
	glUniform1f(uniformLocations[FOG_DISTANCE_LOCATION], fogDistance);
	glUniform3f(uniformLocations[FOG_COLOR_LOCATION], fogColor.x, fogColor.y, fogColor.z);
}


void VRColorShader::draw(const Camera &cam, glm::vec3 lightPos, 
	float fogScale, float fogDistance, glm::vec3 fogColor, Drawable &obj) 
{
	glUseProgram(programID);
	loadUniforms(cam.getProjectionMatrix()*cam.getCameraMatrix(), obj.getTransform(), cam.getPosition(), 
		lightPos, fogScale, fogDistance, fogColor);
	obj.loadUniforms(ShadedMat::id, &uniformLocations[0]);
	obj.loadUniforms(ColorSetMat::id, &uniformLocations[ShadedMat::COUNT]);
	obj.getGeometry().drawGeometry(programID);
	glUseProgram(0);
}

}
