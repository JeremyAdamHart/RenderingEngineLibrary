#pragma once

#include "GLGeometry.h"
#include <glm/glm.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

using namespace glm;

class SimpleGeometry : public GLGeometryContainer {
	GLuint vao;
	size_t bufferSize;
	GLenum mode;

	enum{ POSITION = 0, COUNT };
	GLuint vbo[COUNT];

	bool initializeVAO();

public:
	SimpleGeometry(GLenum mode=GL_TRIANGLES);
	SimpleGeometry(vec3 *positions, size_t elementNum, GLenum mode=GL_TRIANGLES);

	void loadGeometry(vec3 *positions, size_t elementNum);

	void loadPositions(vec3 *positions, size_t numPositions, GLenum usage = GL_STATIC_DRAW);

	virtual void drawGeometry() const;

	//Deprecated?
	virtual void bindGeometry() const;
	virtual int startIndex() const { return 0; }
	virtual int numElements() const { return bufferSize; }
	virtual GLenum getMode() const { return mode; }
	virtual GLint getVaoID() const { return vao; }

	virtual bool usingDrawElements() const { return false; }
};

class SimpleTexGeometry : public GLGeometryContainer {
	GLuint vao;
	size_t bufferSize;
	GLenum mode;
	
	enum{POSITION=0, TEXCOORD, COUNT};
	GLuint vbo[COUNT];

	bool initializeVAO();

public:
	SimpleTexGeometry(GLenum mode=GL_TRIANGLES);
	SimpleTexGeometry(vec3 *positions, vec2 *texCoords, size_t elementNum, GLenum mode=GL_TRIANGLES);

	void loadGeometry(vec3 *positions, vec2 *texCoords, size_t elementNum);

	void loadPositions(vec3 *positions, size_t numPositions, GLenum usage=GL_STATIC_DRAW);
	void loadTexCoords(vec2 *texCoords, size_t numTexCoords, GLenum usage=GL_STATIC_DRAW);

	virtual void drawGeometry() const;

	//Deprecated?
	virtual void bindGeometry() const;
	virtual int startIndex() { return 0; }
	virtual int numElements() { return bufferSize; }
	virtual GLenum getMode() { return mode; }
	virtual GLint getVaoID() { return vao; }

	virtual bool usingDrawElements() { return false; }
};

