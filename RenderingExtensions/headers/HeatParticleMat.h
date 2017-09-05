#pragma once
#include "Material.h"

namespace renderlib {

class HeatParticleMat : public Material {
public:
	enum {
		RADIUS_LOCATION=0,
		COUNT
	};

	static const int id;

	float radius;

	HeatParticleMat(float radius);

	virtual int getType() const;
	virtual void loadUniforms(GLint *locations) const;
};



}