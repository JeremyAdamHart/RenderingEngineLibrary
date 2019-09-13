#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "glSupport.h"
#include "GLObject.h"

using namespace glm;


namespace renderlib {
//Change Arg arg to "Arg *arg"
template<class Arg, class ...Args> 
void loadVBOs(int index, std::vector<GLBuffer> *vbos, size_t bufferSize, GLenum usage, Arg* arg, Args*... args) {
	loadVBOs(index, vbos, bufferSize, usage, arg);
	loadVBOs(index + 1, vbos, bufferSize, usage, args...);
}

template<class Arg> 
void loadVBOs(int index, std::vector<GLBuffer> *vbos, size_t bufferSize, GLenum usage, Arg* arg) {
	glBindBuffer(GL_ARRAY_BUFFER, vbos->at(index));
	glBufferData(GL_ARRAY_BUFFER, bufferSize * sizeof(*arg), arg, usage);
}

//template<class ...Args> void loadVBOs(int index, std::vector<GLuint> *vbos, size_t bufferSize, GLenum usage, Args... args) {}

template<class Arg, class... Args>
bool initVertexBuffers(std::vector<GLBuffer> *vbos, unsigned int attribDivisor=0) {
	return initVertexBuffers<Arg>(vbos, attribDivisor) && initVertexBuffers<Args...>(vbos, attribDivisor);
}
template<> bool initVertexBuffers<vec4>(std::vector<GLBuffer> *vbos, unsigned int attribDivisor);

template<> bool initVertexBuffers<vec3>(std::vector<GLBuffer> *vbos, unsigned int attribDivisor);

template<> bool initVertexBuffers<vec2>(std::vector<GLBuffer> *vbos, unsigned int attribDivisor);

template<> bool initVertexBuffers<unsigned char>(std::vector<GLBuffer> *vbos, unsigned int attribDivisor);

template<> bool initVertexBuffers<float>(std::vector<GLBuffer> *vbos, unsigned int attribDivisor);

template<> bool initVertexBuffers<glm::mat4>(std::vector<GLBuffer> *vbos, unsigned int attribDivisor);

template<class T1, class T2, class... Ts> 
void allocateBufferStorage(GLBuffer* vbo, char* streamed, size_t bufferSize, void** pointers) {
	allocateBufferStorage<T1>(vbo, streamed, bufferSize, pointers);
	allocateBufferStorage<T2, Ts...>(vbo + 1, streamed+1, bufferSize, pointers+1);
}

template<class T> 
void allocateBufferStorage(GLBuffer* vbo, char* streamed, size_t bufferSize, void** pointer) {
	glBindBuffer(GL_ARRAY_BUFFER, (*vbo));
	if (*streamed) {
		GLbitfield flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
		GLbitfield flagsMap = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
		glBufferStorage(GL_ARRAY_BUFFER, bufferSize * sizeof(T), nullptr, flags);
		(*pointer) = glMapBufferRange(GL_ARRAY_BUFFER, 0, bufferSize * sizeof(T), flagsMap);
	}
	else {
		glBufferData(GL_ARRAY_BUFFER, bufferSize * sizeof(T), nullptr, GL_DYNAMIC_DRAW);
		(*pointer) = nullptr;
	}
	checkGLErrors("<allocateBufferStorage>");
}

/*
template<>
void allocateBufferStorage<glm::vec3>(GLuint* vbo, size_t bufferSize, void** pointer) {
	glBindBuffer(GL_ARRAY_BUFFER, (*vbo));
	GLbitfield flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
	glBufferStorage(GL_ARRAY_BUFFER, bufferSize * sizeof(glm::vec3), nullptr, flags);
	(*pointer) = glMapBufferRange(GL_ARRAY_BUFFER, 0, bufferSize * sizeof(glm::vec3), flags);
	glBufferData(GL_ARRAY_BUFFER, bufferSize * sizeof(glm::vec3), nullptr, GL_DYNAMIC_DRAW);
}
*/
//template<>
//void allocateBufferStorage(GLuint* vbo, size_t bufferSize, void** pointers) {}

}

