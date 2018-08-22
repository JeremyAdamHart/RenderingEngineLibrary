#include "Material.h"
#ifndef USING_GLEW
#include <glad/glad.h>
#else
#define GLEW_STATIC
#include <GL/glew.h>
#endif

namespace renderlib {

int Material::getNextID() {
	static int id = 0;
	return ++id;
}

int Material::getType() const { return 0; }
void Material::loadUniforms(GLint *locations) const { }
//int Material::getUniformNum() const { return 0; }
Material::~Material() {}
}
