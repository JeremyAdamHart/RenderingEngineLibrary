#include "VertexBufferTemplates.h"

namespace renderlib {

template<>
unsigned int initVertexBuffers<vec4>(std::vector<GLBuffer> *vbos, unsigned int attribDivisor, unsigned int attributeIndex) {
	GLBuffer vbo = createBufferID();

	int index = attributeIndex;

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

	checkGLErrors("initVertexBuffers");

	return index+1;
}

template<>
unsigned int initVertexBuffers<vec3>(std::vector<GLBuffer> *vbos, unsigned int attribDivisor, unsigned int attributeIndex) {
	GLBuffer vbo = createBufferID();
	
	int index = attributeIndex;

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

	checkGLErrors("initVertexBuffers");

	return index+1;
}

template<>
unsigned int initVertexBuffers<vec2>(std::vector<GLBuffer> *vbos, unsigned int attribDivisor, unsigned int attributeIndex) {
	GLBuffer vbo = createBufferID();

	int index = attributeIndex;

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

	checkGLErrors("initVertexBuffers");

	return index+1;
}
template<> unsigned int initVertexBuffers<float>(std::vector<GLBuffer> *vbos, unsigned int attribDivisor, unsigned int attributeIndex) {
	GLBuffer vbo = createBufferID();

	int index = attributeIndex;

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

	checkGLErrors("initVertexBuffers");

	return index + 1;
}

template<>
unsigned int initVertexBuffers<unsigned char>(std::vector<GLBuffer> *vbos, unsigned int attribDivisor, unsigned int attributeIndex) {
	GLBuffer vbo = createBufferID();

	int index = attributeIndex;

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

	checkGLErrors("<initVertexBuffers>");

	return index + 1;
}

template<> unsigned int initVertexBuffers<glm::mat4>(std::vector<GLBuffer> *vbos, unsigned int attribDivisor, unsigned int attributeIndex) {
	int index = attributeIndex;
	GLBuffer vbo = createBufferID();
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	for (int i = 0; i < 4; i++) {

		glEnableVertexAttribArray(index);
		glVertexAttribPointer(
			index,
			4,
			GL_FLOAT,
			GL_FALSE,
			sizeof(glm::mat4),
			(void*)(i*sizeof(vec4))
		);

		glVertexAttribDivisor(index, attribDivisor);

		index++;
	}

	vbos->push_back(vbo);
	checkGLErrors("initVertexBuffers");

	return index;
}

}