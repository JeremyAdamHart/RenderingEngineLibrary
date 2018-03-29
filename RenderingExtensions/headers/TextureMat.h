#pragma once

#include "Material.h"
#include "Texture.h"

namespace renderlib {

class TextureMat : public Material {
public:
	enum {
		TEXTURE_LOCATION = 0,
		COUNT
	};

	enum {		//Make int enum?
		COLOR=0,
		SHADOW=1 << 16,
		POSITION=2 << 16,
		NORMAL=3 << 16,
		TRANSLUCENT=4 << 16
	};

	static const int id;

	Texture tex;
	int subtype;

	TextureMat(Texture tex, int subtype=COLOR);

	virtual int getType() const;
	virtual void loadUniforms(GLint *locations) const;	//Must have already called useProgram
};

}
