#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>
#include <vector>
#include <map>

using namespace std;

std::string str(const char *fmt, ...);

string fileToString(const string &filename);

GLuint createShader(const string &source, GLenum shaderType);

GLuint createShader(const string &source, const string &defines, GLenum shaderType);

GLuint createGLProgram(GLuint vertexShader, GLuint fragmentShader, GLuint tessControlShader=0, GLuint tessEvalShader=0);

bool checkGLErrors(string location);

GLuint createGLProgram(vector<pair<GLenum, string>> shaders,
	map<GLenum, string> defines = {});



