#pragma once

#include "Texture.h"
#include "Framebuffer.h"
#include <string>

namespace renderlib {

Texture createTexture2D(std::string filename, TextureManager *manager=nullptr);

Texture createTexture2D(TexInfo info, unsigned char *data=nullptr);
Texture createTexture2D(int width, int height, TextureManager *manager);
Texture createTexture2D(TexInfo info, TextureManager *manager, 
	unsigned char *data=nullptr);

template<typename T>
Texture createTexture2DFromData(TexInfo info, TextureManager *manager, T* data) {
	GLTexture texID = createTextureID();

	glActiveTexture(NO_ACTIVE_TEXTURE);	//Bind to avoid disturbing active units
	glBindTexture(GL_TEXTURE_2D, texID);
	//	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_2D, info.level, info.internalFormat, info.dimensions[0],
		info.dimensions[1], 0, info.format, info.type, data);

	return Texture(texID, info, manager);
}

Texture createTexture2DMulti(int width, int height, TextureManager *manager, size_t num_samples);
Texture createTexture2DMulti(TexInfo info, TextureManager *manager, size_t num_samples);

Texture createDepthTexture(int width, int height, TextureManager *manager);
Texture createDepthTextureMulti(int width, int height, TextureManager *manager, size_t num_samples);

Framebuffer createNewFramebuffer(int width, int height);

Framebuffer createFramebufferWithColorAndDepth(int width, int height, TextureManager *manager, size_t num_samples);
Framebuffer createFramebufferWithColorAndDepth(int width, int height, TextureManager *manager);

IndexedFramebuffer createIndexedFramebufferWithColorAndDepth(int width, int height, TextureManager *manager, size_t num_samples);
IndexedFramebuffer createIndexedFramebufferWithColorAndDepth(int width, int height, TextureManager *manager);

void setTextureWrappingAndMipMaps(Texture tex);

}