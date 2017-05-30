#include "ColorMat.h"

const int ColorMat::id = Material::getNextID();

ColorMat::ColorMat(glm::vec3 color) :color(color) {}

int ColorMat::getType() const { return id; }

void ColorMat::loadUniforms(GLint *locations) const {
	glUniform3f(locations[COLOR_LOCATION], color.x, color.y, color.z);
}