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
template<typename GLuint(*cons)(void), typename void(*dest)(GLuint)>
class GLObject {
	GLuint v;
	int* refNum;
	GLObject(GLuint v) :refNum(new int(1)), v(v) {}		//Consider getting rid of

public:
	GLObject() : refNum(new int(1)), v(cons()) {}
	GLObject(const GLObject<cons, dest> &other) :refNum(other.refNum), v(other.v) {
		(*refNum)++;
	}

	GLObject<cons, dest>& operator=(const GLObject<cons, dest> &other) {
		(*refNum)--;
		if ((*refNum) < 1) {
			delete refNum;
			if(v) dest(v);
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
			if(v) dest(v);
		}
	}
};

GLuint createProgramID();
void deleteProgramID(GLuint id);

GLuint createVAOID();
void deleteVAOID(GLuint id);

GLuint createBufferID();
void deleteBufferID(GLuint id);

GLuint createTextureID();
void deleteTextureID(GLuint id);

GLuint createFramebufferID();
void deleteFramebufferID(GLuint id);

using GLProgram = GLObject<&createProgramID, &deleteProgramID>;
using GLVAO = GLObject<&createVAOID, &deleteVAOID>;
using GLBuffer = GLObject<&createBufferID, &deleteBufferID>;
using GLTexture = GLObject<&createTextureID, &deleteTextureID>;
using GLFramebuffer = GLObject<&createFramebufferID, &deleteFramebufferID>;

}