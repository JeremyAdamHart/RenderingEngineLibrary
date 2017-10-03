#pragma once

#include "glSupport.h"

namespace renderlib {

class Shader {
protected:
	GLuint programID;

	Shader();

public:
	GLuint getID();
	void deleteProgram();
	virtual bool createProgram(map<GLenum, string> defines = map<GLenum, string>{});
};

}