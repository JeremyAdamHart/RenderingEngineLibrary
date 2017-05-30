#include "ElementGeometry.h"
#include "glSupport.h"


ElementGeometry::ElementGeometry(GLenum mode) :bufferSize(0), elementNum(0), mode(mode)
{
	initializeVAO();
}

ElementGeometry::ElementGeometry(vec3 *positions, vec3 *normals, vec2 *texCoords, unsigned int *elements,
	size_t bufferSize, size_t elementNum, GLenum mode):
	mode(mode), bufferSize(bufferSize), elementNum(elementNum)
{
	initializeVAO();

	loadGeometry(positions, normals, texCoords, elements, bufferSize, elementNum);
}

void ElementGeometry::loadGeometry(vec3 *positions, vec3 *normals, vec2 *texCoords, unsigned int *elements,
	size_t _bufferSize, size_t _elementNum, GLenum usage)
{
	glBindBuffer(GL_ARRAY_BUFFER, vbo[POSITION]);
	glBufferData(GL_ARRAY_BUFFER, _bufferSize * sizeof(vec3), 
		positions, usage);

	if (normals) {
		glBindBuffer(GL_ARRAY_BUFFER, vbo[NORMAL]);
		glBufferData(GL_ARRAY_BUFFER, _bufferSize * sizeof(vec3), 
			normals, usage);
	}
	if (texCoords) {
		glBindBuffer(GL_ARRAY_BUFFER, vbo[TEXCOORD]);
		glBufferData(GL_ARRAY_BUFFER, _bufferSize * sizeof(vec2), 
			texCoords, usage);
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[ELEMENTS]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, elementNum*sizeof(unsigned int), 
		elements, usage);

	bufferSize = _bufferSize;
	elementNum = _elementNum;
}

bool ElementGeometry::initializeVAO() {
	checkGLErrors("-1");

	glGenVertexArrays(1, &vao);
	glGenBuffers(COUNT, vbo);

	glBindVertexArray(vao);

	checkGLErrors("0");

	glEnableVertexAttribArray(ATTRIB_LOCATION::POSITION);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[POSITION]);
	glVertexAttribPointer(
		ATTRIB_LOCATION::POSITION,			//Attribute
		3,					//# of components
		GL_FLOAT,			//Type
		GL_FALSE,			//Normalized?
		sizeof(vec3),		//Stride
		(void*)0			//Offset
	);

	checkGLErrors("1");

	glEnableVertexAttribArray(ATTRIB_LOCATION::NORMAL);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[NORMAL]);
	glVertexAttribPointer(
		ATTRIB_LOCATION::NORMAL,		//Attribute
		3,					//# of components
		GL_FLOAT,			//Type
		GL_FALSE,			//Normalized?
		sizeof(vec3),		//Stride
		(void*)0			//Offset
	);

	checkGLErrors("2");

	glEnableVertexAttribArray(ATTRIB_LOCATION::TEX_COORD);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[TEXCOORD]);
	glVertexAttribPointer(
		ATTRIB_LOCATION::TEX_COORD,		//Attribute
		2,					//# of components
		GL_FLOAT,			//Type
		GL_FALSE,			//Normalized?
		sizeof(vec2),		//Stride
		(void*)0			//Offset
		);

	glBindVertexArray(0);

	return !checkGLErrors("ElementGeometry::initVao");
}



void ElementGeometry::bindGeometry() const
{
	glBindVertexArray(vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[ELEMENTS]);
}

void ElementGeometry::drawGeometry() const {
	bindGeometry();
	glDrawElements(mode, elementNum, GL_UNSIGNED_INT, 0);
}

int ElementGeometry::startIndex() const { return 0; }
int ElementGeometry::numElements() const { return elementNum; }
GLenum ElementGeometry::getMode() const { return mode; }
GLint ElementGeometry::getVaoID() const { return vao; }

bool ElementGeometry::usingDrawElements() const { return true; }