#include "Shader.h"

namespace renderlib {

using namespace std;

Shader::Shader() :programID(0) {}

GLuint Shader::getID() { return programID; }
void Shader::deleteProgram() { glDeleteProgram(programID); }
bool Shader::createProgram(map<GLenum, string> defines) { return false; }

vector<string> Shader::getUniformNames() { return{}; }

void Shader::calculateUniformLocations() {
	glUseProgram(programID);

	vector<string> uniformNames = getUniformNames();
	for (int i = 0; i < uniformNames.size(); i++) {
		uniformLocations.push_back(glGetUniformLocation(programID, uniformNames[i].c_str()));
	}

	glUseProgram(0);
}

void Shader::loadMaterialUniforms(const Drawable &drawable) {
	for (auto it = materialOffsets.begin(); it != materialOffsets.end(); it++) {
		drawable.loadUniforms(it->first, &uniformLocations[it->second]);
	}
}

}