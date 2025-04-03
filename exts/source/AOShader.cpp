#include "AOShader.h"
#include "ShadedMat.h"
#include "TextureMat.h"
#include "ColorMat.h"

#include <algorithm>

namespace renderlib {

using namespace glm;

enum {
	VP_MATRIX_LOCATION = TextureMat::COUNT + TextureMat::COUNT,
	M_MATRIX_LOCATION,
	CAMERA_POS_LOCATION,
	CAMERA_RIGHT_LOCATION,
	CAMERA_UP_LOCATION,
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

	return programID;
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
	uniformLocations[CAMERA_RIGHT_LOCATION] = glGetUniformLocation(programID,
		"cam_right");
	uniformLocations[CAMERA_UP_LOCATION] = glGetUniformLocation(programID,
		"cam_up");
}

void AOShader::loadUniforms(vec3 camera_pos, vec3 camera_right, vec3 camera_up, vec3 light_pos) {
	glUniform3f(uniformLocations[CAMERA_POS_LOCATION], camera_pos.x, camera_pos.y, camera_pos.z);
	glUniform3f(uniformLocations[CAMERA_RIGHT_LOCATION], camera_pos.x, camera_pos.y, camera_pos.z);
	glUniform3f(uniformLocations[CAMERA_UP_LOCATION], camera_pos.x, camera_pos.y, camera_pos.z);
	glUniform3f(uniformLocations[LIGHT_POS_LOCATION], 
		light_pos.x, light_pos.y, light_pos.z);
}

void AOShader::draw(const Camera &cam, vec3 lightPos, Drawable &obj) {
	glUseProgram(programID);
	vec4 cam_right = cam.getCameraMatrix()*vec4(1, 0, 0, 0);
	vec4 cam_up = cam.getCameraMatrix()*vec4(0, 1, 0, 0);
	loadUniforms(cam.getPosition(), vec3(cam_right.x, cam_right.y, cam_right.z),
		vec3(cam_up.x, cam_up.y, cam_up.z), lightPos);
	
	obj.loadUniforms(TextureMat::id | TextureMat::POSITION, &uniformLocations[0]);
	obj.loadUniforms(TextureMat::id | TextureMat::NORMAL, &uniformLocations[TextureMat::COUNT]);

	obj.getGeometry().drawGeometry(programID);
	glUseProgram(0);
}

}