#pragma once

#include "Material.h"
#include <glm/glm.hpp>

class ColorMat : public Material {
public:
	enum {
		COLOR_LOCATION=0,
		COUNT
	};

	static const int id;

	glm::vec3 color;

	ColorMat(glm::vec3 color=glm::vec3(1.f));

	virtual int getType() const;
	virtual void loadUniforms(GLint *locations) const;	//Must have already called useProgram
};