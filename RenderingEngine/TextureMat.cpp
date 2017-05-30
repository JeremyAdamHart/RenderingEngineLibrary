#include "TextureMat.h"

const int TextureMat::id = Material::getNextID();

TextureMat::TextureMat(Texture tex) :tex(tex) {}

int TextureMat::getType() const {return id;}

void TextureMat::loadUniforms(GLint *locations) const {
//	GLenum unit = tex.getTexUnit();
//	int unitNum = unit - GL_TEXTURE0;
	glUniform1i(locations[TEXTURE_LOCATION], tex.getTexUnit()-GL_TEXTURE0);
}