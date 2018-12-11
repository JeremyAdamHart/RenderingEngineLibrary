#pragma once
#ifndef USING_GLEW
#include <glad/glad.h>
#else
#define GLEW_STATIC
#include <GL/glew.h>
#endif

#include "GLObject.h"
#include <string>
#include <vector>
#include <map>

namespace renderlib {

using namespace std;

std::string str(const char *fmt, ...);

string fileToString(const string &filename);

GLuint createShader(const string &source, GLenum shaderType);

GLuint createShader(const string &source, const string &defines, GLenum shaderType);

GLProgram createGLProgram(GLuint vertexShader, GLuint fragmentShader, GLuint tessControlShader=0, GLuint tessEvalShader=0);

bool checkGLErrors(string location);

GLProgram createGLProgram(vector<pair<GLenum, string>> shaders,
	map<GLenum, string> defines = map<GLenum, string>{});

}
