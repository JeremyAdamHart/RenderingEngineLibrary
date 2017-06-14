#include "AOShader.h"
#include "ShadedMat.h"
#include "TextureMat.h"
#include "ColorMat.h"

#include <algorithm>

using namespace glm;

enum {
	VP_MATRIX_LOCATION = TextureMat::COUNT + TextureMat::COUNT,
	M_MATRIX_LOCATION,
	CAMERA_POS_LOCATION,
	LIGHT_POS_LOCATION,
	COUNT
};

static vector<pair<GLenum, string>> shaders{
	{ GL_VERTEX_SHADER, "shaders/ambient.vert" },
	{ GL_FRAGMENT_SHADER, "shaders/ambient.frag" }
};

AOShader::AOShader(map<GLenum, string> defines)
{
	createProgram(defines);
	calculateUniformLocations();
}

bool AOShader::createProgram(map<GLenum, string> defines) {
	
	programID = createGLProgram(shaders, defines);

	return programID != 0;
}

void AOShader::calculateUniformLocations() {
	glUseProgram(programID);

	//Material uniforms
	uniformLocations.resize(COUNT);

	
	uniformLocations[TextureMat::TEXTURE_LOCATION] =
		glGetUniformLocation(programID, "positionTex");
	uniformLocations[TextureMat::TEXTURE_LOCATION + TextureMat::COUNT] =
		glGetUniformLocation(programID, "normalTex");

	//Other uniforms
	uniformLocations[CAMERA_POS_LOCATION] = glGetUniformLocation(programID,
		"camera_position");
	uniformLocations[LIGHT_POS_LOCATION] = glGetUniformLocation(programID,
		"lightPos");
}

void AOShader::loadUniforms(vec3 camera_pos, vec3 light_pos) {
	glUniform3f(uniformLocations[CAMERA_POS_LOCATION], camera_pos.x, camera_pos.y, camera_pos.z);
	glUniform3f(uniformLocations[LIGHT_POS_LOCATION], 
		light_pos.x, light_pos.y, light_pos.z);
}

void AOShader::draw(const Camera &cam, vec3 lightPos, const Drawable &obj) {
	glUseProgram(programID);
	loadUniforms(cam.getPosition(), lightPos);
	
	obj.loadUniforms(TextureMat::id | TextureMat::POSITION, &uniformLocations[0]);
	obj.loadUniforms(TextureMat::id | TextureMat::NORMAL, &uniformLocations[TextureMat::COUNT]);

	obj.getGeometry().drawGeometry();
	glUseProgram(0);
}