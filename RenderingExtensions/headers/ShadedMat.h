#pragma once

#include "Material.h"
#include "Texture.h"

class ShadedMat : public Material {
public:
	enum {
		KA_LOCATION=0,
		KD_LOCATION,
		KS_LOCATION,
		ALPHA_LOCATION,
		COUNT
	};

	static const int id;

	float ka;
	float kd;
	float ks;
	float alpha;

	ShadedMat(float ka, float kd, float ks, float alpha);

	virtual int getType() const;
	virtual void loadUniforms(GLint *locations) const;	//Must have already called useProgram
};