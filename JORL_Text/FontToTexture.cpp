#include "FontToTexture.h"
#include "TextureCreation.h"
#include <vector>

namespace renderlib {

using namespace glm;

CharBitmap::CharBitmap() :width(0), height(0) {}
CharBitmap::CharBitmap(unsigned int width, unsigned int height):width(width), height(height){}

Font createFontTexture(FT_Library* ft, const char* fontFilename, TextureManager* texManager) {

	int resolutionX = 16;
	int resolutionY = 16;

	FT_Face face;
	FT_New_Face(*ft, fontFilename, 0, &face);
	FT_Set_Pixel_Sizes(face,
		resolutionX, resolutionY);

	Font font;

	unsigned int maxBitmapWidth = 0;
	unsigned int maxBitmapHeight = 0;

	for (unsigned char i = 20; i < 126; i++) {
		FT_Load_Char(face, i, FT_LOAD_DEFAULT);	//FT_LOAD_MONOCHROME);
		FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);

		FT_GlyphSlot glyph = face->glyph;
		FT_Glyph_Metrics& metrics = face->glyph->metrics;

		float scaleToNormalized = 1.f / (64.f*float(resolutionX));
		Character c;
		c.horizontalAdvance = float(metrics.horiAdvance)*scaleToNormalized;
		c.verticalAdvance = float(metrics.vertAdvance)*scaleToNormalized;
		c.width = float(metrics.width)*scaleToNormalized;
		c.height = float(metrics.height)*scaleToNormalized;
		c.textTopLeftFromHorizontalOrigin = glm::vec2(
			float(metrics.horiBearingX),
			float(metrics.horiBearingY)
		)*scaleToNormalized;

		c.textTopLeftFromVerticalOrigin = glm::vec2(
			float(metrics.vertBearingX),
			float(metrics.vertBearingY)
		)*scaleToNormalized;


		//Turn bitmap into texture
		unsigned char *inputBuffer = glyph->bitmap.buffer;
	
		c.bitmap.data.resize(glyph->bitmap.width*glyph->bitmap.rows);
		c.bitmap.width = glyph->bitmap.width;
		c.bitmap.height = glyph->bitmap.rows;

		maxBitmapWidth = max(maxBitmapWidth, c.bitmap.width);
		maxBitmapHeight = max(maxBitmapHeight, c.bitmap.height);
		
		//printf("Pixel mode = %d, rows = %d, columns = %d, row offset = %d\n", glyph->bitmap.pixel_mode, glyph->bitmap.rows, glyph->bitmap.width, glyph->bitmap.pitch);

		size_t rowStart = 0; 
		for (size_t y = 0; y < glyph->bitmap.rows; y++) {
			for (size_t x = 0; x < glyph->bitmap.width; x++) {
				c.bitmap.data[y*glyph->bitmap.width + x] = inputBuffer[rowStart + x];
			}
			rowStart += glyph->bitmap.pitch;
		}

		font.charInfo[i] = c;

		//Pack into 9x12x(64x64) texture
		
		if (i == 'c') {
			Texture tex = createTexture2D(
				TexInfo(
					GL_TEXTURE_2D,
					{ int(glyph->bitmap.width), int(glyph->bitmap.rows) },
					0,
					GL_RED,
					GL_R8, 
					GL_UNSIGNED_BYTE),
				texManager,
				c.bitmap.data.data());

			font.tex = tex;

		}	
	}

	//@TODO Rename to rows/columns?
	unsigned int tableBlockWidth = 9;
	unsigned int tableBlockHeight = 12;
	unsigned int tableWidth = tableBlockWidth * resolutionX;
	unsigned int tableHeight = tableBlockHeight * resolutionY;

	float blockWidth = 1.f / float(tableBlockWidth);
	float blockHeight = 1.f / float(tableBlockHeight);

	std::vector<unsigned char> bitmapTable;
	bitmapTable.resize(tableWidth*tableHeight, 0);

	unsigned char characterOffset = 20;
	for (unsigned char i = 0; i < 106; i++) {
		unsigned int tableBlockIndexX = i % tableBlockWidth;
		unsigned int tableBlockIndexY = i / tableBlockWidth;

		printf("(%d, %d)\n", tableBlockIndexX, tableBlockIndexY);

		unsigned int tableBlockIndex = tableBlockIndexY * tableBlockWidth + tableBlockIndexX;

		Character& c = font.charInfo[i+characterOffset];

		for (size_t y = 0; y < c.bitmap.height; y++) {
			for (size_t x = 0; x < c.bitmap.width; x++) {
				size_t indexY = tableBlockIndexY * resolutionY + y;
				size_t indexX = tableBlockIndexX * resolutionX + x;
				size_t index = indexY * tableWidth + indexX;

				bitmapTable[index] = c.bitmap.data[y*c.bitmap.width + x];
			}
		}

		//Calculate uv coordinate
		vec2 topLeftUV = vec2(float(tableBlockIndexX), float(tableBlockIndexY));


		//TL
		c.pointCoords[0] = c.textTopLeftFromHorizontalOrigin;
		c.uvCoords[0] = (vec2(float(tableBlockIndexX), float(tableBlockIndexY)) + vec2(0, c.height))*vec2(blockWidth, blockHeight);
		//TR
		c.pointCoords[1] = c.textTopLeftFromHorizontalOrigin + vec2(c.width, 0);
		c.uvCoords[1] = (topLeftUV + vec2(0, c.height))*vec2(blockWidth, blockHeight);
		//BR
		c.pointCoords[2] = c.textTopLeftFromHorizontalOrigin + vec2(c.width, -c.height);
		c.uvCoords[2] = (topLeftUV + vec2(c.width, 0))*vec2(blockWidth, blockHeight);
		//BL
		c.pointCoords[3] = c.textTopLeftFromHorizontalOrigin + vec2(0, -c.height);
		c.uvCoords[3] = (topLeftUV + vec2(0, 0))*vec2(blockWidth, blockHeight);

	}

	Texture tex = createTexture2D(
		TexInfo(
			GL_TEXTURE_2D,
			{ int(tableWidth), int(tableHeight) },
			0,
			GL_RED,
			GL_R8,
			GL_UNSIGNED_BYTE),
		texManager,
		bitmapTable.data());

	font.tex = tex;

	return font;
}

Character& Font::character(unsigned char c) {
	return charInfo[c];
}



}