#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "glSupport.h"

using namespace glm;


namespace renderlib {
//Change Arg arg to "Arg *arg"
template<class Arg, class ...Args> 
void loadVBOs(int index, std::vector<GLuint> *vbos, size_t bufferSize, GLenum usage, Arg* arg, Args*... args) {
	loadVBOs(index, vbos, bufferSize, usage, arg);
	loadVBOs(index + 1, vbos, bufferSize, usage, args...);
}

template<class Arg> 
void loadVBOs(int index, std::vector<GLuint> *vbos, size_t bufferSize, GLenum usage, Arg* arg) {
	glBindBuffer(GL_ARRAY_BUFFER, vbos->at(index));
	glBufferData(GL_ARRAY_BUFFER, bufferSize * sizeof(*arg), arg, usage);
}

//template<class ...Args> void loadVBOs(int index, std::vector<GLuint> *vbos, size_t bufferSize, GLenum usage, Args... args) {}

template<class Arg, class... Args>
bool initVertexBuffers(std::vector<GLuint> *vbos) {
	return initVertexBuffers<Arg>(vbos) && initVertexBuffers<Args...>(vbos);
}

template<> bool initVertexBuffers<vec3>(std::vector<GLuint> *vbos) {
	GLuint vbo;
	glGenBuffers(1, &vbo);

	int index = vbos->size();

	glEnableVertexAttribArray(index);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(
		index,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(vec3),
		(void*)0
	);

	vbos->push_back(vbo);

	return !checkGLErrors("initVertexBuffers");
}

template<> bool initVertexBuffers<vec2>(std::vector<GLuint> *vbos) {
	GLuint vbo;
	glGenBuffers(1, &vbo);

	int index = vbos->size();

	glEnableVertexAttribArray(index);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(
		index,
		2,
		GL_FLOAT,
		GL_FALSE,
		sizeof(vec2),
		(void*)0
	);

	vbos->push_back(vbo);

	return !checkGLErrors("initVertexBuffers");
}

template<> bool initVertexBuffers<char>(std::vector<GLuint> *vbos) {
	GLuint vbo;
	glGenBuffers(1, &vbo);

	int index = vbos->size();

	glEnableVertexAttribArray(index);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(
		index,
		1,
		GL_BYTE,
		GL_FALSE,
		sizeof(char),
		(void*)0
	);

	vbos->push_back(vbo);

	return !checkGLErrors("initVertexBuffers");
}

template<class T1, class T2, class... Ts> 
void allocateBufferStorage(GLuint* vbo, size_t bufferSize, void** pointers) {
	allocateBufferStorage<T1>(vbo, bufferSize, pointers);
	allocateBufferStorage<T2, Ts...>(vbo + 1, bufferSize, pointers+1);
}

template<class T> 
void allocateBufferStorage(GLuint* vbo, size_t bufferSize, void** pointer) {
	glBindBuffer(GL_ARRAY_BUFFER, (*vbo));
	GLbitfield flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
	glBufferStorage(GL_ARRAY_BUFFER, bufferSize * sizeof(T), nullptr, flags);
	(*pointer) = glMapBufferRange(GL_ARRAY_BUFFER, 0, bufferSize * sizeof(T), flags);
}
//template<>
//void allocateBufferStorage(GLuint* vbo, size_t bufferSize, void** pointers) {}

}

