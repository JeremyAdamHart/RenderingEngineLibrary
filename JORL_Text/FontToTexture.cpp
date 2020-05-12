#include "FontToTexture.h"
#include "TextureCreation.h"
#include <vector>

namespace renderlib {

using namespace glm;

CharBitmap::CharBitmap() :width(0), height(0) {}
CharBitmap::CharBitmap(unsigned int width, unsigned int height):width(width), height(height){}

Font createFont(FT_Library* ft, const char* fontFilename, TextureManager* texManager, unsigned int resolution) {

	int resolutionX = resolution;
	int resolutionY = resolution;

	FT_Face face;
	FT_New_Face(*ft, fontFilename, 0, &face);
	FT_Set_Pixel_Sizes(face,
		resolutionX, resolutionY);

	Font font;

	unsigned int maxBitmapWidth = 0;
	unsigned int maxBitmapHeight = 0;

	font.ascend = face->size->metrics.ascender / (64.f*float(resolutionY));
	font.descend = face->size->metrics.descender / (64.f*float(resolutionY));
	
	for (unsigned char i = 32; i < 127; i++) {
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

	int padding = 1;

	maxBitmapWidth+= padding;
	maxBitmapHeight+= padding;

	//@TODO Rename to rows/columns?
	unsigned int tableBlockWidth = 8;
	unsigned int tableBlockHeight = 12;
	unsigned int tableWidth = tableBlockWidth * maxBitmapWidth;
	unsigned int tableHeight = tableBlockHeight * maxBitmapHeight;

	float blockWidth = 1.f / float(tableBlockWidth);
	float blockHeight = 1.f / float(tableBlockHeight);

	std::vector<unsigned char> bitmapTable;
	bitmapTable.resize(tableWidth*tableHeight, 0);

	unsigned char characterOffset = 32;
	for (unsigned char i = 0; i < 95; i++) {
		unsigned int tableBlockIndexX = i % tableBlockWidth;
		unsigned int tableBlockIndexY = i / tableBlockWidth;

		printf("(%d, %d)\n", tableBlockIndexX, tableBlockIndexY);

		unsigned int tableBlockIndex = tableBlockIndexY * tableBlockWidth + tableBlockIndexX;

		Character& c = font.charInfo[i+characterOffset];

		for (size_t y = 0; y < c.bitmap.height; y++) {
			for (size_t x = 0; x < c.bitmap.width; x++) {
				size_t indexY = tableBlockIndexY * maxBitmapHeight + y;
				size_t indexX = tableBlockIndexX * maxBitmapWidth + x;
				size_t index = indexY * tableWidth + indexX;

				bitmapTable[index] = c.bitmap.data[y*c.bitmap.width + x];
			}
		}

		float charWidthUV = float(c.bitmap.width) / float(tableWidth);
		float charHeightUV = float(c.bitmap.height) / float(tableHeight);

		vec2 widthOffset(float(padding) / float(tableWidth), float(padding)/float(tableHeight));
		widthOffset.y = 0.f;

		//Calculate uv coordinate
		vec2 topLeftUV = vec2(float(tableBlockIndexX), float(tableBlockIndexY));
		topLeftUV *= vec2(blockWidth, blockHeight);

		//TL
		c.pointCoords[0] = c.textTopLeftFromHorizontalOrigin;
		c.uvCoords[0] = topLeftUV + vec2(0, 0);
		//TR
		c.pointCoords[1] = c.textTopLeftFromHorizontalOrigin + vec2(c.width, 0);
		c.uvCoords[1] = topLeftUV + vec2(charWidthUV, 0);
		//BR
		c.pointCoords[2] = c.textTopLeftFromHorizontalOrigin + vec2(c.width, -c.height);
		c.uvCoords[2] = topLeftUV + vec2(charWidthUV, charHeightUV);
		//BL
		c.pointCoords[3] = c.textTopLeftFromHorizontalOrigin + vec2(0, -c.height);
		c.uvCoords[3] = topLeftUV + vec2(0, charHeightUV);

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

const Character& Font::character(unsigned char c) const { return charInfo.at(c); }

void getTextBuffers(const char* text, const Font& font, glm::vec3 offset, float scale, std::vector<glm::vec3>* points, std::vector<glm::vec3>* offsets, std::vector<glm::vec2>* uvs, std::vector<unsigned int>* faces) {
	int t_index = 0;
	float horizontalOffset = 0.f;
	while (text[t_index] != '\0') {
		const Character& c = font.character(text[t_index]);
		for (int i = 0; i < 4; i++) {
			points->push_back(glm::vec3((c.pointCoords[i] +vec2(horizontalOffset, 0))*scale, 0.f));
			offsets->push_back(offset);
			uvs->push_back(c.uvCoords[i]);
		}

		unsigned int faceIndex0 = points->size() - 4;
		faces->push_back(faceIndex0);
		faces->push_back(faceIndex0 + 1);
		faces->push_back(faceIndex0 + 2);
		faces->push_back(faceIndex0);
		faces->push_back(faceIndex0 + 2);
		faces->push_back(faceIndex0 + 3);

		horizontalOffset += c.horizontalAdvance;
		t_index++;
	}
}

void getTextBuffers(const char* text, const Font& font, std::vector<glm::vec3>* points, std::vector<glm::vec3>* offsets, std::vector<glm::vec2>* uvs, std::vector<unsigned int>* faces) {
	getTextBuffers(text, font, vec3(0), 1.f, points, offsets, uvs, faces);
}


}