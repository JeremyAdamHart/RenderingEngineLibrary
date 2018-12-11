#include "ColorMat.h"

namespace renderlib {

const int ColorMat::id = Material::getNextID();

ColorMat::ColorMat(glm::vec4 color) :color(color) {}
ColorMat::ColorMat(glm::vec3 newColor) : color(newColor, 1.f) {}

int ColorMat::getType() const { return id; }

void ColorMat::loadUniforms(GLint *locations) const {
	glUniform4f(locations[COLOR_LOCATION], color.x, color.y, color.z, color.a);
}

//int ColorMat::getUniformNum() const { return COUNT; }

}