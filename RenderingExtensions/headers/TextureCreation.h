#pragma once

#include "Texture.h"
#include "Framebuffer.h"
#include <string>

Texture createTexture2D(std::string filename, TextureManager *manager=nullptr);

Texture createTexture2D(TexInfo info, unsigned char *data=nullptr);
Texture createTexture2D(int width, int height, TextureManager *manager);
Texture createTexture2D(TexInfo info, TextureManager *manager, 
	unsigned char *data=nullptr);

Texture createTexture2DMulti(int width, int height, TextureManager *manager, size_t num_samples);
Texture createTexture2DMulti(TexInfo info, TextureManager *manager, size_t num_samples);

Texture createDepthTexture(int width, int height, TextureManager *manager);
Texture createDepthTextureMulti(int width, int height, TextureManager *manager, size_t num_samples);

Framebuffer createNewFramebuffer(int width, int height);