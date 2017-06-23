#include "ProcessingShader.h"

ProcessingShader::ProcessingShader(string fragmentShader, string defines):
	shaders({{GL_VERTEX_SHADER, "processing.vert"}, {GL_FRAGMENT_SHADER, fragmentShader}})
{
	createProgram({ { GL_FRAGMENT_SHADER, defines } });
}

bool ProcessingShader::createProgram(map<GLenum, string> defines) {

	programID = createGLProgram(shaders, defines);

	return programID != 0;
}

void ProcessingShader::draw(const Drawable &obj) {
	glUseProgram(programID);
	obj.getGeometry().drawGeometry();
	glUseProgram(0);
}