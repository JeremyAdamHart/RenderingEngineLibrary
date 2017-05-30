#pragma once

#include "glSupport.h"

class Shader {
protected:
	GLuint programID;

	Shader();

public:
	GLuint getID();
	void deleteProgram();
	virtual bool createProgram(map<GLenum, string> defines = {});
};