#include "TorranceSparrow.h"
#include <glm/glm.hpp>

GLint TorranceSparrow::program = 0;

using namespace glm;

TorranceSparrow::TorranceSparrow()
{
	if (program == 0)
		initializeShader();
}

void TorranceSparrow::initializeShader()
{
	cout << "Compiling tsShaded.vert" << endl;
	GLuint vertexShader = createShader(fileToString("shaders/tsShaded.vert"), GL_VERTEX_SHADER);
	cout << "Compiling tsShaded.frag" << endl;
	GLuint fragmentShader = createShader(fileToString("shaders/tsShaded.frag"), GL_FRAGMENT_SHADER);
	program = createProgram(vertexShader, fragmentShader, 0, 0);
}

GLint TorranceSparrow::getProgram() { return program; }
void TorranceSparrow::deleteProgram() { glDeleteProgram(program); }

void TorranceSparrow::loadUniforms()
{
	glUseProgram(program);
	//glActiveTexture(GL_TEXTURE0 + texUnit);
	//glBindTexture(GL_TEXTURE_2D, texID);

	GLint uniformLocation = glGetUniformLocation(program, "colorTexture");
	if (uniformLocation == -1)
		cout << "Could not find uniform" << endl;
	glUniform1i(uniformLocation, texUnit);
}

