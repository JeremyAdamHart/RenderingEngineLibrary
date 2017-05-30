#include "Material.h"
#include <glad/glad.h>

int Material::getNextID() {
	static int id = 0;
	return ++id;
}

int Material::getType() const { return 0; }
void Material::loadUniforms(GLint *locations) const { }
