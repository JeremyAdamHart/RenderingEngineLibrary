#include "Shader.h"
#include "glSupport.h"

namespace renderlib {

Shader::Shader() :programID(0) {}

GLuint Shader::getID() { return programID; }
void Shader::deleteProgram() { if(programID != 0) glDeleteProgram(programID); }
bool Shader::createProgram(map<GLenum, string> defines) { return false; }
bool Shader::createNewProgram(vector<pair<GLenum, string>> shaderNames, map<GLenum, string> defines) {
	programID = createGLProgram(shaderNames, defines);
	return programID != 0;
}

}