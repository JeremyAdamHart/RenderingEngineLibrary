#pragma once

#include "GLGeometry.h"
#include "GLObject.h"
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <vector>

namespace renderlib {

using namespace glm;

class SimpleGeometry : public GLGeometryContainer {
	GLVAO vao;
	size_t bufferSize;
	GLenum mode;

	enum{ POSITION = 0, COUNT };
	std::vector<GLBuffer> vbo;

	bool initializeVAO();

public:
	SimpleGeometry(GLenum mode=GL_TRIANGLES);
	SimpleGeometry(vec3 *positions, size_t elementNum, GLenum mode=GL_TRIANGLES);

	void loadGeometry(vec3 *positions, size_t elementNum);

	void loadPositions(vec3 *positions, size_t numPositions, GLenum usage = GL_STATIC_DRAW);

	virtual void drawGeometry();
	virtual GLuint getVboID(int num) const { return (num < COUNT) ? vbo[num] : 0; }

	//Deprecated?
	virtual void bindGeometry() const;
	virtual int startIndex() const { return 0; }
	virtual int numElements() const { return bufferSize; }
	virtual GLenum getMode() const { return mode; }
	virtual GLuint getVaoID() const { return vao; }

	virtual bool usingDrawElements() const { return false; }
};

class SimpleTexGeometry : public GLGeometryContainer {

	GLVAO vao;
	size_t bufferSize;
	GLenum mode;

	GLenum texCoordType;
	
	enum{POSITION=0, TEXCOORD, COUNT};
	std::vector<GLBuffer> vbo;

	bool initializeVAO();

public:
	SimpleTexGeometry(GLenum mode=GL_TRIANGLES, GLenum type=GL_FLOAT);
	SimpleTexGeometry(vec3 *positions, vec2 *texCoords, size_t elementNum, GLenum mode=GL_TRIANGLES);
	SimpleTexGeometry(vec3 *positions, ivec2 *texCoords, size_t elementNum, GLenum mode = GL_TRIANGLES);

	void loadGeometry(vec3 *positions, vec2 *texCoords, size_t elementNum);
	void loadGeometry(vec3 *positions, ivec2 *texCoords, size_t elementNum);

	void loadPositions(vec3 *positions, size_t numPositions, GLenum usage=GL_STATIC_DRAW);
	void loadTexCoords(vec2 *texCoords, size_t numTexCoords, GLenum usage=GL_STATIC_DRAW);
	void loadTexCoords(ivec2 *texCoords, size_t numTexCoords, GLenum usage = GL_STATIC_DRAW);

	virtual void drawGeometry();

	virtual GLuint getVboID(int num) const { return (num < COUNT) ? vbo[num] : 0; }

	//Deprecated?
	virtual void bindGeometry() const;
	virtual int startIndex() { return 0; }
	virtual int numElements() { return bufferSize; }
	virtual GLenum getMode() { return mode; }
	virtual GLuint getVaoID() { return vao; }

	virtual bool usingDrawElements() { return false; }
};

class SimpleTexGeometryI : public GLGeometryContainer {
	GLVAO vao;
	size_t bufferSize;
	GLenum mode;

	enum { POSITION = 0, TEXCOORD, COUNT };
	std::vector<GLBuffer> vbo;

	bool initializeVAO();

public:
	SimpleTexGeometryI(GLenum mode = GL_TRIANGLES);
	SimpleTexGeometryI(vec3 *positions, ivec2 *texCoords, size_t elementNum, GLenum mode = GL_TRIANGLES);

	void loadGeometry(vec3 *positions, ivec2 *texCoords, size_t elementNum);
	void loadGeometry(vec3 *positions, GLint *texCoords, size_t elementNum);

	void loadPositions(vec3 *positions, size_t numPositions, GLenum usage = GL_STATIC_DRAW);
	void loadTexCoords(ivec2 *texCoords, size_t numTexCoords, GLenum usage = GL_STATIC_DRAW);
	
	virtual void drawGeometry();

	//Deprecated?
	virtual void bindGeometry() const;
	virtual int startIndex() { return 0; }
	virtual int numElements() { return bufferSize; }
	virtual GLenum getMode() { return mode; }
	virtual GLuint getVaoID() { return vao; }

	virtual bool usingDrawElements() { return false; }
};

}