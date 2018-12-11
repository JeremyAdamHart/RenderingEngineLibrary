#include "VertexBufferTemplates.h"

namespace renderlib {

template<>
bool initVertexBuffers<vec3>(std::vector<GLuint> *vbos) {
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

template<>
bool initVertexBuffers<vec2>(std::vector<GLuint> *vbos) {
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

template<>
bool initVertexBuffers<unsigned char>(std::vector<GLuint> *vbos) {
	GLuint vbo;
	glGenBuffers(1, &vbo);

	int index = vbos->size();

	glEnableVertexAttribArray(index);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribIPointer(
		index,
		1,
		GL_UNSIGNED_BYTE,
		sizeof(unsigned char),
		(void*)0
	);

	vbos->push_back(vbo);

	return !checkGLErrors("<initVertexBuffers>");
}

}