#include "VertexBufferTemplates.h"

namespace renderlib {

template<>
int initVertexAttribute<vec4>(const GLBuffer& vbo, unsigned int attribDivisor, int attributeIndex) {

	int index = attributeIndex;

	if (index < 0)
		return index;

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

	checkGLErrors("initVertexAttribute");

	return index+1;
}

template<>
int initVertexAttribute<vec3>(const GLBuffer& vbo, unsigned int attribDivisor, int attributeIndex) {
	
	int index = attributeIndex;

	if (index < 0)
		return index;

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

	checkGLErrors("initVertexAttribute");

	return index+1;
}

template<>
int initVertexAttribute<vec2>(const GLBuffer& vbo, unsigned int attribDivisor, int attributeIndex) {

	int index = attributeIndex;

	if (index < 0)
		return index;

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

	checkGLErrors("initVertexAttribute");

	return index+1;
}
template<> 
int initVertexAttribute<float>(const GLBuffer& vbo, unsigned int attribDivisor, int attributeIndex) {
	int index = attributeIndex;

	if (index < 0)
		return index;

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

	checkGLErrors("initVertexAttribute");

	return index + 1;
}

template<>
int initVertexAttribute<unsigned char>(const GLBuffer& vbo, unsigned int attribDivisor, int attributeIndex) {
	int index = attributeIndex;

	if (index < 0)
		return index;

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

	checkGLErrors("<initVertexAttribute>");

	return index + 1;
}

template<> 
int initVertexAttribute<glm::mat4>(const GLBuffer& vbo, unsigned int attribDivisor, int attributeIndex) {
	int index = attributeIndex;

	if (index < 0)
		return index;

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

	checkGLErrors("initVertexBuffers");

	return index;
}

}