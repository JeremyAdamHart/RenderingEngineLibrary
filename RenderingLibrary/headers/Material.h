#pragma once
#include <iostream>
#ifndef USING_GLEW
#include <glad/glad.h>
#else
#define GLEW_STATIC
#include <GL/glew.h>
#endif

namespace renderlib{

//All derived subclasses are singletons
class Material{
protected:
	static int getNextID();

	Material(){}
public:
	virtual int getType() const;
	virtual void loadUniforms(GLint *locations) const;
};

}