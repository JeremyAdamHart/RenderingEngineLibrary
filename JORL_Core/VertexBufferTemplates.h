#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "glSupport.h"
#include "GLObject.h"
#include "VertexAttribLayout.h"

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

template<class T>
int initVertexAttribute(const GLBuffer& vbo, unsigned int attribDivisor, int attributeIndex) {
	//static_assert(false, "initVertexAttribute<T>::Unsupported type");
}

template<class Arg>
unsigned int initVertexBuffers(std::vector<GLBuffer> *vbos, unsigned int attribDivisor = 0, unsigned int attributeIndex = 0) {
	int index = attributeIndex;
	GLBuffer vbo = createBufferID();
	vbos->push_back(vbo);
	return initVertexAttribute<Arg>(vbo, attribDivisor, attributeIndex);
}

template<class Arg1, class Arg2, class... Args>
unsigned int initVertexBuffers(std::vector<GLBuffer> *vbos, unsigned int attribDivisor=0, unsigned int attributeIndex=0) {
	return initVertexBuffers<Arg2, Args...>(vbos, attribDivisor, 
		initVertexBuffers<Arg1>(vbos, attribDivisor, attributeIndex));
}

template<> int initVertexAttribute<vec4>(const GLBuffer& vbo, unsigned int attribDivisor, int attributeIndex);

template<> int initVertexAttribute<vec3>(const GLBuffer& vbo, unsigned int attribDivisor, int attributeIndex);

template<> int initVertexAttribute<vec2>(const GLBuffer& vbo, unsigned int attribDivisor, int attributeIndex);

template<> int initVertexAttribute<unsigned char>(const GLBuffer& vbo, unsigned int attribDivisor, int attributeIndex);

template<> int initVertexAttribute<float>(const GLBuffer& vbo, unsigned int attribDivisor, int attributeIndex);

template<> int initVertexAttribute<glm::mat4>(const GLBuffer& vbo, unsigned int attribDivisor, int attributeIndex);

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

template<class T1, class T2, class... Ts>
void allocateBufferStorage(GLBuffer* vbo, size_t bufferSize, void** pointers) {
	allocateBufferStorage<T1>(vbo, bufferSize, pointers);
	allocateBufferStorage<T2, Ts...>(vbo + 1, bufferSize, pointers + 1);
}

template<class T>
void allocateBufferStorage(GLBuffer* vbo, size_t bufferSize, void** pointer) {
	glBindBuffer(GL_ARRAY_BUFFER, (*vbo));
	if (attrib::usingPinned<T>()) {
		GLbitfield flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
		GLbitfield flagsMap = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
		glNamedBufferStorage(*vbo, bufferSize * sizeof(T), nullptr, flags);
		(*pointer) = glMapNamedBufferRange((*vbo), 0, bufferSize * sizeof(T), flagsMap);
	}
	else {
		glBufferData(GL_ARRAY_BUFFER, bufferSize * sizeof(T), nullptr, GL_DYNAMIC_DRAW);
		(*pointer) = nullptr;
	}
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	checkGLErrors("createBufferStorage");
}

//Need to add divisor to Position attribute
template<class T>
void initVertexAttributes(GLBuffer *vbos, int* attribLocations) {
	initVertexAttribute<typename T::Type>(*vbos, T::Divisor, attribLocations[0]);
}

template<class T1, class T2, class... Ts>
void initVertexAttributes(GLBuffer *vbos, int* attribLocations) {
	initVertexAttributes<T1>(vbos, attribLocations);
	initVertexAttributes<T2, Ts...>(vbos+1, attribLocations+1);
}

/*template<class T>
unsigned int initVertexBindings_imp(std::vector<GLBuffer> *vbos, unsigned int bufferIndex) {
	GLBuffer vbo = createBufferID();

	int index = bufferIndex;
	glBindVertexBuffer(index, vbo, 0, sizeof(T));
	vbos->push_back(vbo);

	checkGLErrors("initVertexBuffers");
	return index++;
}

template<class T>
unsigned int initVertexBindings(std::vector<GLBuffer> *vbos, unsigned int bufferIndex = 0) {
	return initVertexBindings_imp<typename T::Type>(vbos, bufferIndex);
}

template<class T1, class T2, class... Ts>
unsigned int initVertexBindings(std::vector<GLBuffer> *vbos, unsigned int bufferIndex = 0) {
	return initVertexBindings<T2, Ts...>(vbos, 
		initVertexBindings<T1>(vbos, bufferIndex));
}


template<typename T>
void initVertexAttribute_imp(std::vector<GLBuffer> *vbos, unsigned int attributeIndex);
*/
/*
template<> void initVertexAttribute_imp<vec4>(std::vector<GLBuffer> *vbos, unsigned int attribDivisor, unsigned int attributeIndex);
template<> void initVertexAttribute_imp<vec3>(std::vector<GLBuffer> *vbos, unsigned int attribDivisor, unsigned int bufferIndex);
template<> void initVertexAttribute_imp<vec2>(std::vector<GLBuffer> *vbos, unsigned int attribDivisor, unsigned int bufferIndex);
template<> void initVertexAttribute_imp<unsigned char>(std::vector<GLBuffer> *vbos, unsigned int attribDivisor, unsigned int bufferIndex);
template<> void initVertexAttribute_imp<float>(std::vector<GLBuffer> *vbos, unsigned int attribDivisor, unsigned int bufferIndex);
template<> void initVertexAttribute_imp<glm::mat4>(std::vector<GLBuffer> *vbos, unsigned int attribDivisor, unsigned int bufferIndex);
*/

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

