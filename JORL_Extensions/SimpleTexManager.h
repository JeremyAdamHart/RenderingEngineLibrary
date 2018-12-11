#pragma once

#include "Texture.h"
#include <map>

namespace renderlib {

class SimpleTexManager : public TextureManager{
	map<int, Texture> textures;
	vector<int> texUnits;

	int counter;
	int nextUnit;

public:
	SimpleTexManager();

	virtual void removeTexture(int handle);
	virtual int addTexture(Texture tex);
	virtual int requestTexUnit(int handle);
};

int sizeOfTexture(const Texture &tex);

}