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

///
/// The base class of all geometry classes. All classes must implement the drawGeometry function.
///
class GLGeometryContainer {
protected:
	GLGeometryContainer() {}

public:
	virtual void drawGeometry() {}
	///
	/// Binds vertex array object and calls appropriate draw function
	/// @param programID - The program being used to draw this geometry
	virtual void drawGeometry(GLProgram programID) {}

	virtual ~GLGeometryContainer() {}
};

}