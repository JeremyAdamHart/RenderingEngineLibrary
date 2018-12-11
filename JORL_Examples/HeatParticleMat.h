#pragma once
#include "Material.h"

class HeatParticleMat : public renderlib::Material {
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
