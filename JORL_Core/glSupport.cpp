#include "glSupport.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <stdarg.h>

//#include <glad/glad.h>

/*
Following function found from
https://gist.github.com/dgiagio/b0187dac64b60e6ff626
created by Diego Giagio
*/

namespace renderlib {

std::string str(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	char buf[32];
	size_t n = std::vsnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);

	// Static buffer large enough?
	if (n < sizeof(buf)) {
		return{ buf, n };
	}

	// Static buffer too small
	std::string s(n + 1, 0);
	va_start(args, fmt);
	std::vsnprintf(const_cast<char*>(s.data()), s.size(), fmt, args);
	va_end(args);

	return s;
}

string fileToString(const string &filename)
{
	ifstream file(filename.c_str());

	if (file.is_open()){
		string contents((istreambuf_iterator<char>(file)),
			(istreambuf_iterator<char>()));
		file.close();
		return contents;
	}
	else{
		cout << filename.c_str() << " not found" << endl;
		return string();
	}
}

GLuint createShader(const string &source, GLenum shaderType){

	GLuint shader = glCreateShader(shaderType);

	const GLchar *source_ptr = source.c_str();
	glShaderSource(shader, 1, &source_ptr, NULL);
	glCompileShader(shader);

	GLint success = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

	if (success == GL_FALSE){
		int logSize = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize);

		string errorMessage(logSize, ' ');
		glGetShaderInfoLog(shader, logSize, NULL, &errorMessage[0]);

		cout << source << endl;
		cout << "[Errors detected]" << endl << errorMessage.c_str() << endl;

		glDeleteShader(shader);

		return 0;
	}
	else
		return shader;

	checkGLErrors("createShader");
}

GLuint createShader(const string &source, const string &defines, GLenum shaderType) {

	GLuint shader = glCreateShader(shaderType);

	size_t index = source.find('\n');

	if (index == string::npos) {
		glDeleteShader(shader);
		return 0;
	}
	
	string version = source.substr(0, index+1);
	string content = source.substr(index + 1);

	string newSource = version + defines + content;

//	cout << newSource.c_str() << endl;

	const GLchar *source_ptr = newSource.c_str();
	glShaderSource(shader, 1, &source_ptr, NULL);
	glCompileShader(shader);

	GLint success = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

	if (success == GL_FALSE) {
		int logSize = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize);

		string errorMessage(logSize, ' ');
		glGetShaderInfoLog(shader, logSize, NULL, &errorMessage[0]);

		cout << newSource << endl;
		cout << "[Errors detected]" << endl << errorMessage.c_str() << endl;

		glDeleteShader(shader);

		return 0;
	}
	else
		return shader;

	checkGLErrors("createShader");
}

GLProgram createGLProgram(GLuint vertexShader, GLuint fragmentShader, GLuint tessControlShader, GLuint tessEvalShader)
{
	GLProgram program = createProgramID();

	if (vertexShader)	glAttachShader(program, vertexShader);
	if (fragmentShader)	glAttachShader(program, fragmentShader);
	if (tessControlShader) glAttachShader(program, tessControlShader);
	if (tessEvalShader) glAttachShader(program, tessEvalShader);
	
	glLinkProgram(program);

	GLint isLinked = 0;
	glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
	if (isLinked == GL_FALSE)
	{
		int logSize = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logSize);

		string errorMessage(logSize, ' ');
		glGetProgramInfoLog(program, logSize, NULL, &errorMessage[0]);

		cout << "[Linking failed]" << endl << errorMessage.c_str() << endl;

		//glDeleteProgram(program);

		return GLProgram::wrap(0);
	}

	if (vertexShader) glDetachShader(program, vertexShader);
	if (fragmentShader) glDetachShader(program, fragmentShader);
	if (tessControlShader) glDetachShader(program, tessControlShader);
	if (tessEvalShader) glDetachShader(program, tessEvalShader);

	return program;

	checkGLErrors("createProgram");
}

GLProgram createGLProgram(vector<pair<GLenum, string>> shaders,
	map<GLenum, string> defines) 
{
	GLProgram program = createProgramID();

	vector<GLuint> shaderIDs(shaders.size());

	for (int i = 0; i < shaderIDs.size(); i++) {
		GLenum shaderType = shaders[i].first;
		string shaderName = shaders[i].second;
		cout << "Compiling " << shaderName << endl;
		try{
			string define = defines.at(shaders[i].first);
			shaderIDs[i] = createShader(fileToString(shaderName), 
				define, shaderType);
		}
		catch (out_of_range) {
			shaderIDs[i] = createShader(fileToString(shaderName), 
										shaderType);
		}

		if (shaderIDs[i])
			glAttachShader(program, shaderIDs[i]);
		else
			return GLProgram::wrap(0);
	}

	glLinkProgram(program);

	GLint isLinked = 0;
	glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
	if (isLinked == GL_FALSE)
	{
		int logSize = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logSize);

		string errorMessage(logSize, ' ');
		glGetProgramInfoLog(program, logSize, NULL, &errorMessage[0]);

		cout << "[Linking failed]" << endl << errorMessage.c_str() << endl;

		//glDeleteProgram(program);

		return GLProgram::wrap(0);
	}

	//Clean up shaders
	for (int i = 0; i < shaderIDs.size(); i++) {
		if (shaderIDs[i]) {
			glDetachShader(program, shaderIDs[i]);
			glDeleteShader(shaderIDs[i]);
		}
	}

	checkGLErrors("createProgram");

	return program;
}

bool checkGLErrors(string location){
#ifdef NDEBUG		//TODO - Switch to Callbacks?
	bool error = false;
	for (GLenum flag = glGetError(); flag != GL_NO_ERROR; flag = glGetError())
	{
		cout << "<" << location << ">: ";
		switch (flag) {
		case GL_INVALID_ENUM:
			cout << "GL_INVALID_ENUM" << endl; break;
		case GL_INVALID_VALUE:
			cout << "GL_INVALID_VALUE" << endl; break;
		case GL_INVALID_OPERATION:
			cout << "GL_INVALID_OPERATION" << endl; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			cout << "GL_INVALID_FRAMEBUFFER_OPERATION" << endl; break;
		case GL_OUT_OF_MEMORY:
			cout << "GL_OUT_OF_MEMORY" << endl; break;
		default:
			cout << "[unknown error code]" << endl;
		}
		error = true;
	}
	return error;
#else
	return false;
#endif
}

}
