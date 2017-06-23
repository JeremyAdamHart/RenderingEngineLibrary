#pragma once

#include <string>
#include <vector>
#include "Drawable.h"

bool loadWavefront(std::string filename, std::vector<Drawable> *drawables);