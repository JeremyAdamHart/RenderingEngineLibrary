#pragma once

#ifndef USING_GLEW
#include <glad/glad.h>
#else
#define GLEW_STATIC
#include <GL/glew.h>
#endif
#include <vector>
#include <string>
#include <map>
#include <memory>

#include <VertexAttribLayout.h>


namespace renderlib {

//Namespace for vertex attributes
struct ATTRIB_LOCATION {
	enum{
		POSITION=0,
		NORMAL,
		TEX_COORD,
		COLOR,
			};
};

struct stringMapCompare {
	bool operator()(const char* a, const char* b) const;
};

/**
**For maximum reusability, use consistent naming for vertex attributes:
**	VertexPosition
**	VertexNormal
**	VertexTexCoord
**	VertexColor
*/
class VertexAttribLayout {
public:
	std::map<const char*, int, stringMapCompare> attribMap;
	VertexAttribLayout();
	VertexAttribLayout(GLuint program, std::vector<const char*> attributeNames);

	bool operator==(const VertexAttribLayout& b) const;
	bool operator!=(const VertexAttribLayout& b) const;

	int& operator[](const char* attribName);		//Should this exist?
	const int &operator[](const char* attribName) const;

	bool isComplete() const;
};

/*
** Links programs to correct VAO that supports matches program's layout indices
** Update with new GLuint wrapper
*/
class VertexArraySelector {	// : public std::map<Key, std::shared_ptr<Value>>{
public:
	VertexArraySelector();

	virtual GLuint getVAO(GLuint program);		//Returns 0 if not assigned
	virtual bool isKnownProgram(GLuint program);
	virtual bool registerLayout(GLuint program, const VertexAttribLayout& layout);	//Return true if layout already existed
	virtual bool hasValidVAO(GLuint program);
	virtual void registerVAO(GLuint program, GLuint vao);
protected:
	//Implementation subject to change
	unsigned int nextMapValue;
	std::map<GLuint, int> programMap;
	std::map<int, VertexAttribLayout> layouts;
	std::map<int, GLuint> vaoMap;
};

class GLGeometryContainer {
protected:
	GLGeometryContainer() {}
	VertexArraySelector vaSelector;

public:
	virtual void drawGeometry() {}
	virtual void drawGeometry(GLProgram programID) {}

	virtual void bindGeometry() {}
	virtual int startIndex() const { return 0; }
	virtual int numElements() const { return 0; }
	virtual GLenum getMode() const { return 0; }

	virtual GLuint getVboID(int num) const { return 0; }
	virtual GLuint getVaoID() const { return 0; }		//Don't use to bind VAO, only for comparison

	virtual bool usingDrawElements() const { return false; }

	virtual ~GLGeometryContainer() {}
};

}