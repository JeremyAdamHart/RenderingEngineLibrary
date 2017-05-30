#pragma once
#include <glm/glm.hpp>

#include "glSupport.h"
#include "Material.h"

class TorranceSparrow : public Material {
	static GLint program;
	void initializeShader();
public:
	TorranceSparrow();

	int texUnit;
	GLuint texID;

	virtual GLint getProgram();
	virtual void loadUniforms();
	void deleteProgram();
};
