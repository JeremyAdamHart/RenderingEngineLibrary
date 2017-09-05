#include "Shader.h"

namespace renderlib {

Shader::Shader() :programID(0) {}

GLuint Shader::getID() { return programID; }
void Shader::deleteProgram() { glDeleteProgram(programID); }
bool Shader::createProgram(map<GLenum, string> defines) { return false; }

}