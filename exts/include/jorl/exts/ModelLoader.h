#pragma once

#include <string>
#include <vector>
#include <jorl/core/Drawable.h>
#include <jorl/core/Texture.h>

namespace renderlib {

bool loadWavefront(std::string directory, std::string filename, 
	std::vector<Drawable> *drawables, TextureManager *manager);

}