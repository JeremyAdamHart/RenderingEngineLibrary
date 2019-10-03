#pragma once
#include "Texture.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include <freetype/freetype.h>
#include <map>
#include "glmSupport.h"

namespace renderlib {

class CharBitmap {
public:
	CharBitmap();
	CharBitmap(unsigned int width, unsigned int height);

	std::vector<unsigned char> data;
	unsigned int width;
	unsigned int height;
};

class Character {
public:
	CharBitmap bitmap;

	float horizontalAdvance;
	float verticalAdvance;
	float width;
	float height;
	glm::vec2 textTopLeftFromHorizontalOrigin;
	glm::vec2 textTopLeftFromVerticalOrigin;
	glm::vec2 pointCoords[4];
	glm::vec2 uvCoords [4];

};

class Font {
public:
	Texture tex;
	std::map<char, Character> charInfo;

	Character& character(unsigned char c);
};

Font createFontTexture(FT_Library* ft, const char* fontFilename, TextureManager* texManager);
}
