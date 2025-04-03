#include "Shader.h"
#include "glSupport.h"

namespace renderlib {

using namespace std;

Shader::Shader() :programID(GLProgram::wrap(0)) {}
Shader::Shader(map<int, int> materialOffsets) : programID(GLProgram::wrap(0)), materialOffsets(materialOffsets) {}
Shader::Shader(int materialID) : programID(GLProgram::wrap(0)), materialOffsets({ {materialID, 0} }) {}

/*Shader::Shader(vector<int> materialIDs) : programID(0) {
	int totalOffset = 0;
	for (int i = 0; i < materialIDs.size(); i++) {
		materialOffsets[materialIDs[i]] = totalOffset;
		totalOffset += 
	}
}*/

GLProgram Shader::getID() { return programID; }
void Shader::deleteProgram() { glDeleteProgram(programID); }
bool Shader::createProgram(map<GLenum, string> defines) { return false; }
/*bool Shader::createNewProgram(vector<pair<GLenum, string>> shaderNames, map<GLenum, string> defines) {
	programID = createGLProgram(shaderNames, defines);
	return programID != 0;
}
*/
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