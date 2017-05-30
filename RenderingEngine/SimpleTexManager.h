#pragma once

#include "Texture.h"
#include <map>

class SimpleTexManager : public TextureManager{
	map<int, Texture> textures;
	vector<int> texUnits;

	int counter;
	int nextUnit;

public:
	SimpleTexManager();

	virtual void removeTexture(int handle);
	virtual int addTexture(Texture tex);
	virtual GLenum requestTexUnit(int handle);
};

int sizeOfTexture(const Texture &tex);