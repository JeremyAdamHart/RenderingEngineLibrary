#pragma once

#include "Texture.h"
#include "Framebuffer.h"
#include <string>

Texture createTexture2D(std::string filename, TextureManager *manager=nullptr);

Texture createTexture2D(TexInfo info, unsigned char *data=nullptr);
Texture createTexture2D(int width, int height, TextureManager *manager);
Texture createTexture2D(TexInfo info, TextureManager *manager, 
	unsigned char *data=nullptr);
Texture createDepthTexture(int width, int height, TextureManager *manager);

Framebuffer createNewFramebuffer(int width, int height);