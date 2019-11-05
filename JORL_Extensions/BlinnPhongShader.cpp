#include "BlinnPhongShader.h"
#include "ShadedMat.h"
#include "TextureMat.h"
#include "ColorMat.h"

#include <algorithm>

using namespace glm;

namespace renderlib {

constexpr int cMax(int a, int b) {
	return (a > b) ? a : b;
}

enum {
	VP_MATRIX_LOCATION = ShadedMat::COUNT
	+ cMax(int(TextureMat::COUNT), int(ColorMat::COUNT)),
	M_MATRIX_LOCATION,
	CAMERA_POS_LOCATION,
	LIGHT_POS_LOCATION,
	COUNT
};

static vector<pair<GLenum, string>> shaders{
	{ GL_VERTEX_SHADER, "shaders/bpShaded.vert" },
	{ GL_FRAGMENT_SHADER, "shaders/bpShaded.frag" }
};

//TEST SHADER vv
BlinnPhongShaderT::BlinnPhongShaderT() :
	ShaderT<ShadedMat, ColorMat>(shaders, {},
		{ "ka", "kd", "ks", "alpha", "color",
			"view_projection_matrix",  "model_matrix", "camera_position", "lightPos" })
{}

void BlinnPhongShaderT::draw(const Camera &cam, vec3 lightPos,
	Drawable &obj)
{
	glUseProgram(programID);

	mat4 vp_matrix = cam.getProjectionMatrix()*cam.getCameraMatrix();
	mat4 m_matrix = obj.getTransform();
	vec3 camera_pos = cam.getPosition();

	loadMaterialUniforms(obj);
	glUniformMatrix4fv(uniformLocations[VP_MATRIX_LOCATION], 1, false, &vp_matrix[0][0]);
	glUniformMatrix4fv(uniformLocations[M_MATRIX_LOCATION], 1, false, &m_matrix[0][0]);
	glUniform3f(uniformLocations[CAMERA_POS_LOCATION], camera_pos.x, camera_pos.y, camera_pos.z);
	glUniform3f(uniformLocations[LIGHT_POS_LOCATION],
		lightPos.x, lightPos.y, lightPos.z);
	obj.getGeometry().drawGeometry(programID);
	glUseProgram(0);
}

void BlinnPhongShaderT::drawVertexBinding(const Camera& cam, glm::vec3 lightPos, Drawable &obj)
{
	glUseProgram(programID);

	mat4 vp_matrix = cam.getProjectionMatrix()*cam.getCameraMatrix();
	mat4 m_matrix = obj.getTransform();
	vec3 camera_pos = cam.getPosition();

	loadMaterialUniforms(obj);
	glUniformMatrix4fv(uniformLocations[VP_MATRIX_LOCATION], 1, false, &vp_matrix[0][0]);
	glUniformMatrix4fv(uniformLocations[M_MATRIX_LOCATION], 1, false, &m_matrix[0][0]);
	glUniform3f(uniformLocations[CAMERA_POS_LOCATION], camera_pos.x, camera_pos.y, camera_pos.z);
	glUniform3f(uniformLocations[LIGHT_POS_LOCATION],
		lightPos.x, lightPos.y, lightPos.z);
	obj.getGeometry().drawGeometry(programID);
	glUseProgram(0);
}

//TEST SHADER ^^

BlinnPhongShader::BlinnPhongShader(BPTextureUsage texUsage):
	usingTexture(texUsage == BPTextureUsage::TEXTURE)
{
	map<GLenum, string> defines = (usingTexture) ?
		map<GLenum, string>{ {GL_FRAGMENT_SHADER, "#define USING_TEXTURE\n"} } :
		map<GLenum, string>{};

	createProgram(defines);
	calculateUniformLocations();
}

BlinnPhongShader::BlinnPhongShader(map<GLenum, string> defines) :
	usingTexture(false)
{
	createProgram(defines);
	calculateUniformLocations();
}

bool BlinnPhongShader::createProgram(map<GLenum, string> defines) {
	
	try {
		if (defines.at(GL_FRAGMENT_SHADER).find("#define USING_TEXTURE")
			!= string::npos)
			usingTexture = true;
	}catch (out_of_range) {}

	programID = createGLProgram(shaders, defines);

	return programID;
}


void BlinnPhongShader::calculateUniformLocations() {
	glUseProgram(programID);

	//Material uniforms
	uniformLocations.resize(COUNT);

	uniformLocations[ShadedMat::KA_LOCATION] = glGetUniformLocation(programID, "ka");
	uniformLocations[ShadedMat::KD_LOCATION] = glGetUniformLocation(programID, "kd");
	uniformLocations[ShadedMat::KS_LOCATION] = glGetUniformLocation(programID, "ks");
	uniformLocations[ShadedMat::ALPHA_LOCATION] = glGetUniformLocation(programID, "alpha");

	if (usingTexture)
		uniformLocations[TextureMat::TEXTURE_LOCATION + ShadedMat::COUNT] =
			glGetUniformLocation(programID, "colorTexture");
	else
		uniformLocations[ColorMat::COLOR_LOCATION + ShadedMat::COUNT] =
			glGetUniformLocation(programID, "color");

	//Other uniforms
	uniformLocations[VP_MATRIX_LOCATION] = glGetUniformLocation(programID,
		"view_projection_matrix");
	uniformLocations[M_MATRIX_LOCATION] = glGetUniformLocation(programID,
		"model_matrix");
	uniformLocations[CAMERA_POS_LOCATION] = glGetUniformLocation(programID,
		"camera_position");
	uniformLocations[LIGHT_POS_LOCATION] = glGetUniformLocation(programID,
		"lightPos");
}

void BlinnPhongShader::loadUniforms(const mat4& vp_matrix, 
	const mat4& m_matrix, vec3 camera_pos, vec3 light_pos) {
	glUniformMatrix4fv(uniformLocations[VP_MATRIX_LOCATION], 1, false, &vp_matrix[0][0]);
	glUniformMatrix4fv(uniformLocations[M_MATRIX_LOCATION], 1, false, &m_matrix[0][0]);
	glUniform3f(uniformLocations[CAMERA_POS_LOCATION], camera_pos.x, camera_pos.y, camera_pos.z);
	glUniform3f(uniformLocations[LIGHT_POS_LOCATION], 
		light_pos.x, light_pos.y, light_pos.z);
}


void BlinnPhongShader::draw(const Camera &cam, vec3 lightPos, 
	Drawable &obj) {
	glUseProgram(programID);
	loadUniforms(cam.getProjectionMatrix()*cam.getCameraMatrix(), 
		obj.getTransform(), cam.getPosition(), lightPos);
	
	if (usingTexture)
		obj.loadUniforms(TextureMat::id, &uniformLocations[ShadedMat::COUNT]);
	else
		obj.loadUniforms(ColorMat::id, &uniformLocations[ShadedMat::COUNT]);

	obj.loadUniforms(ShadedMat::id, &uniformLocations[0]);

	obj.getGeometry().drawGeometry(programID);
 	glUseProgram(0);
}

}