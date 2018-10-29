#pragma once

#ifndef USING_GLEW
#include <glad/glad.h>
#else
#define GLEW_STATIC
#include <GL/glew.h>
#endif

#include <cstdio>

namespace renderlib {

//Todo? Make constructor as free function
//Template arguments are creation and deletion functions
//Use this instead of GLuint handles
//Can be passed directly to OpenGL functions via implicit conversion
template<typename void(*dest)(GLuint)>
class GLObject {
	GLuint v;
	int* refNum;
	GLObject() : refNum(new int(1)), v(0) {}

public:
	GLObject(GLuint v) :refNum(new int(1)), v(v) {}		//Consider getting rid of
	GLObject(const GLObject<dest> &other) :refNum(other.refNum), v(other.v) {
		(*refNum)++;
	}

	GLObject<dest>& operator=(const GLObject<dest> &other) {
		(*refNum)--;
		if ((*refNum) < 1) {
			delete refNum;
			if(v) 
				dest(v);
		}
		refNum = other.refNum;
		v = other.v;
		(*refNum)++;

		return (*this);
	}

	operator bool() const {
		return v != 0;
	}

	operator GLuint() const {
		return v;
	}

	//operator GLuint() {
	//	return v;
	//}

	void print() const {
		printf("ID - %d | Num - %d\n", v, (*refNum));
	}

	static GLObject wrap(GLuint id) {
		return GLObject(id);
	}

	~GLObject() {
		(*refNum)--;
		if ((*refNum) < 1) {
			delete refNum;
			if(v) 
				dest(v);
		}
	}
};

void deleteProgramID(GLuint id);
void deleteShaderID(GLuint id);
void deleteVAOID(GLuint id);
void deleteBufferID(GLuint id);
void deleteTextureID(GLuint id);
void deleteFramebufferID(GLuint id);

using GLProgram = GLObject<&deleteProgramID>;
using GLShader = GLObject<&deleteShaderID>;
using GLVAO = GLObject<&deleteVAOID>;
using GLBuffer = GLObject<&deleteBufferID>;
using GLTexture = GLObject<&deleteTextureID>;
using GLFramebuffer = GLObject<&deleteFramebufferID>;

GLProgram createProgramID();
GLShader createShaderID(GLenum type);
GLVAO createVAOID();
GLBuffer createBufferID();
GLTexture createTextureID();
GLFramebuffer createFramebufferID();
}