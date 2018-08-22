#pragma once
#include <iostream>
#ifndef USING_GLEW
#include <glad/glad.h>
#else
#define GLEW_STATIC
#include <GL/glew.h>
#endif

namespace renderlib{

class Material{
protected:
	static int getNextID();

	Material(){}
public:
	virtual int getType() const;
//	virtual int getUniformNum() const;
	virtual void loadUniforms(GLint *locations) const;

	virtual ~Material();
};

}