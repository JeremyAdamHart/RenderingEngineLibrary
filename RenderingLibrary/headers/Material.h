#pragma once
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

//All derived subclasses are singletons
class Material{
protected:
	static int getNextID();

	Material(){}
public:
	virtual int getType() const;
	virtual void loadUniforms(GLint *locations) const;
};