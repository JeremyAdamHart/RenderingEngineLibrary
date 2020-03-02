#pragma once

#include "Bitmask.h"
#include "Material.h"
#include <glm/glm.hpp>
#include <vector>

namespace renderlib {

class ColorSetMat : public Material {
public:
	enum {
		COLORS_LOCATION=0,
		VISIBILITY_LOCATION,
		COUNT
	};

	static const int id;

	Bitmask visibility;
	std::vector<glm::vec3> colors;

	ColorSetMat(std::vector<glm::vec3> colors);

	virtual  int getType() const override;
	virtual void loadUniforms(GLint *locations) const override;
};


}