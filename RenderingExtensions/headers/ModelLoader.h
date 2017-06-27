#pragma once

#include <string>
#include <vector>
#include "Drawable.h"
#include "Texture.h"

bool loadWavefront(std::string directory, std::string filename, 
	std::vector<Drawable> *drawables, TextureManager *manager);