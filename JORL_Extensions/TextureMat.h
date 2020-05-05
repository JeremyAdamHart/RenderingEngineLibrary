#pragma once

#include "Material.h"
#include "Texture.h"

namespace renderlib {

struct ColorTag {};
struct NormalMapTag {};

template<typename T>
class TextureMatT : public Material {
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
	unsigned int subtype;

	TextureMatT(Texture tex, unsigned int subtype=COLOR) :tex(tex), subtype(subtype){}

	virtual int getType() const override { return id | subtype;  };
	virtual void loadUniforms(GLint *locations) const override {
		glUniform1i(locations[TEXTURE_LOCATION], tex.getTexUnit());
	}
};

template<typename T>
const int TextureMatT<T>::id = Material::getNextID();

using TextureMat = TextureMatT<ColorTag>;
using NormalMapMat = TextureMatT<NormalMapTag>;

}
