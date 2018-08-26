#pragma once

#include "GLGeometry.h"
#include "GLObject.h"
#include <glm/glm.hpp>
#include <vector>

namespace renderlib {

using namespace glm;

class ElementGeometry : public GLGeometryContainer {
	GLVAO vao;
	size_t bufferSize;
	size_t elementNum;

	enum { POSITION = 0, NORMAL, TEXCOORD, ELEMENTS, COUNT };
	std::vector<GLBuffer> vbo;

	bool initializeVAO();

public:
	GLenum mode;

	ElementGeometry(GLenum mode = GL_TRIANGLES);
	ElementGeometry(vec3 *positions, vec3 *normals, vec2 *texCoords, unsigned int *elements,
		size_t bufferSize, size_t elementNum, GLenum mode = GL_TRIANGLES);

	void loadGeometry(vec3 *positions, vec3 *normals, vec2 *texCoords, unsigned int *elements,
		size_t _bufferSize, size_t _elementNum, GLenum usage = GL_STATIC_DRAW);

	virtual void drawGeometry() const;

	//Deprecated?
	virtual void bindGeometry() const;
	virtual int startIndex() const;
	virtual int numElements() const;
	virtual GLenum getMode() const;
	virtual GLuint getVaoID() const;

	virtual bool usingDrawElements() const;
};

}