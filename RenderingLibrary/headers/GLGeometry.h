#pragma once

#ifndef USING_GLEW
#include <glad/glad.h>
#else
#define GLEW_STATIC
#include <GL/glew.h>
#endif

//Namespace for vertex attributes
struct ATTRIB_LOCATION {
	enum{
		POSITION=0,
		NORMAL,
		TEX_COORD,
		COLOR,
			};
};

class GLGeometryContainer{
protected:
	GLGeometryContainer(){}

public:
	virtual void drawGeometry()const {}

	virtual void bindGeometry() const {}
	virtual int startIndex() const { return 0; }
	virtual int numElements() const { return 0; }
	virtual GLenum getMode() const { return 0; }
	virtual GLint getVaoID() const { return -1; }		//Don't use to bind VAO, only for comparison

	virtual bool usingDrawElements() const { return false; }
};