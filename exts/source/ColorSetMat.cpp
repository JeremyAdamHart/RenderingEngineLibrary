#include "ColorSetMat.h"

namespace renderlib {

const int ColorSetMat::id = Material::getNextID();

ColorSetMat::ColorSetMat(std::vector<glm::vec3> colors) :colors(colors) {}

int ColorSetMat::getType() const { return id; }

void ColorSetMat::loadUniforms(GLint *locations) const {
	glUniform3fv(locations[COLORS_LOCATION], colors.size(), &colors[0][0]);
	int bitmapWordCount = colors.size() / 32 + 1;
	glUniform1uiv(locations[VISIBILITY_LOCATION], bitmapWordCount, visibility.getWords());
}



}