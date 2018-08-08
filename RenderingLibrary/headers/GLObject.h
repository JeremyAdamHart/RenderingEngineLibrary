#pragma once

#ifndef USING_GLEW
#include <glad/glad.h>
#else
#define GLEW_STATIC
#include <GL/glew.h>
#endif

//Template arguments are creation and deletion functions
template<typename GLuint (*cons)(void), typename void (*dest)(GLuint)>
class GLObject {
public:
	GLuint v;
	int* refNum;

	GLObject() : refNum(new int(1)), v(cons()){}
//	GLObject(GLuint v) :refNum(new(1)), v(v) {}		//Consider getting rid of
	GLObject(const GLObject<cons, dest> &other) :refNum(other.refNum), v(other.v) {
		(*refNum)++;
	}

	GLObject<cons, dest>& operator=(const GLObject<cons, dest> &other) {
		refNum = other.refNum;
		v = other.v;
		(*refNum)++;
	}

	operator GLuint&() const {
		return v;
	}

	~GLObject() {
		(*refNum)--;
		delete refNum;
		if ((*refNum) < 1)
			dest(v);
	}
};

GLuint createProgramID() { return glCreateProgram(); }
void deleteProgramID(GLuint id) { glDeleteProgram(id); }

GLuint createVAOID() { GLuint vaoID;  glGenVertexArrays(1, &vaoID); return vaoID; }
void deleteVAOID(GLuint id) { glDeleteVertexArrays(1, &id); }

GLuint createBufferID() { GLuint bufferID; glGenBuffers(1, &bufferID); return bufferID; }
void deleteBufferID(GLuint id) { glDeleteBuffers(1, &id); }

GLuint createTextureID() { GLuint textureID; glGenTextures(1, &textureID); return textureID; }
void deleteTextureID(GLuint id) { glDeleteTextures(1, &id); }

GLuint createFramebufferID() { GLuint framebufferID; glGenFramebuffers(1, &framebufferID); return framebufferID; }
void deleteFramebufferID(GLuint id) { glDeleteFramebuffers(1, &id); }

using GLProgram = GLObject<&createProgramID, &deleteProgramID>;
using GLVAO = GLObject<&createVAOID, &deleteVAOID>;
using GLBuffer = GLObject<&createBufferID, &deleteBufferID>;
using GLTexture = GLObject<&createTextureID, &deleteTextureID>;
using GLFramebufferID = GLObject<&createFramebufferID, &deleteFramebufferID>;