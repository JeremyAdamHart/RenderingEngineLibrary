#pragma once

#include "Material.h"
#include "Texture.h"

class TextureMat : public Material {
public:
	enum {
		TEXTURE_LOCATION = 0,
		COUNT
	};

	static const int id;

	Texture tex;

	TextureMat(Texture tex);

	virtual int getType() const;
	virtual void loadUniforms(GLint *locations) const;	//Must have already called useProgram
};