#include "VertexBufferTemplates.h"

namespace renderlib {

template<>
bool initVertexBuffers<vec4>(std::vector<GLBuffer> *vbos, unsigned int attribDivisor) {
	GLBuffer vbo = createBufferID();

	int index = vbos->size();

	glEnableVertexAttribArray(index);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(
		index,
		4,
		GL_FLOAT,
		GL_FALSE,
		sizeof(vec4),
		(void*)0
	);

	glVertexAttribDivisor(index, attribDivisor);

	vbos->push_back(vbo);

	return !checkGLErrors("initVertexBuffers");
}

template<>
bool initVertexBuffers<vec3>(std::vector<GLBuffer> *vbos, unsigned int attribDivisor) {
	GLBuffer vbo = createBufferID();
	
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

	glVertexAttribDivisor(index, attribDivisor);

	vbos->push_back(vbo);

	return !checkGLErrors("initVertexBuffers");
}

template<>
bool initVertexBuffers<vec2>(std::vector<GLBuffer> *vbos, unsigned int attribDivisor) {
	GLBuffer vbo = createBufferID();

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

	glVertexAttribDivisor(index, attribDivisor);

	vbos->push_back(vbo);

	return !checkGLErrors("initVertexBuffers");
}
template<> bool initVertexBuffers<float>(std::vector<GLBuffer> *vbos, unsigned int attribDivisor) {
	GLBuffer vbo = createBufferID();

	int index = vbos->size();

	glEnableVertexAttribArray(index);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(
		index,
		1,
		GL_FLOAT,
		GL_FALSE,
		sizeof(float),
		(void*)0
	);

	glVertexAttribDivisor(index, attribDivisor);

	vbos->push_back(vbo);

	return !checkGLErrors("initVertexBuffers");
}

template<>
bool initVertexBuffers<unsigned char>(std::vector<GLBuffer> *vbos, unsigned int attribDivisor) {
	GLBuffer vbo = createBufferID();

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

	glVertexAttribDivisor(index, attribDivisor);

	vbos->push_back(vbo);

	return !checkGLErrors("<initVertexBuffers>");
}

template<> bool initVertexBuffers<glm::mat4>(std::vector<GLBuffer> *vbos, unsigned int attribDivisor) {
	GLBuffer vbo = createBufferID();

	int index = vbos->size();

	glEnableVertexAttribArray(index);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(
		index,
		16,
		GL_FLOAT,
		GL_FALSE,
		sizeof(glm::mat4),
		(void*)0
	);

	glVertexAttribDivisor(index, attribDivisor);

	vbos->push_back(vbo);

	return !checkGLErrors("initVertexBuffers");
}

}