#include "SimpleTexManager.h"
#include <iostream>
#include <limits>

namespace renderlib {

SimpleTexManager::SimpleTexManager():counter(0), nextUnit(0) {
	int maxTexUnits = 0;
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTexUnits);
	cout << "Max available texture units = " << maxTexUnits << endl;

	texUnits.resize(maxTexUnits - 1, -1);
}

void SimpleTexManager::removeTexture(int handle) {
	textures.erase(handle);
	for (int i = 0; i < texUnits.size(); i++) {
		if (texUnits[i] == handle)
			texUnits[i] = -1;
	}
}

int SimpleTexManager::addTexture(Texture tex) {
	textures[++counter] = tex;
	return counter;
}

//Change -> Use a most recent method
int SimpleTexManager::requestTexUnit(int handle) {
	//If already bound
	for (int i = 0; i < texUnits.size(); i++) {
		if (texUnits[i] == handle) {
			return i + 1;
		}
	}

	int texUnit = nextUnit;
	nextUnit = ++nextUnit % texUnits.size();
	glActiveTexture(GL_TEXTURE1 + texUnit);
	glBindTexture(textures[handle].getTarget(), textures[handle].getID());

	texUnits[texUnit] = handle;
	glActiveTexture(NO_ACTIVE_TEXTURE);
	return texUnit + 1;	//Because we don't use 0
}

int sizeOfTexture(const Texture &tex) {
	int size = 1;
	if (tex.getWidth() > 0) size *= tex.getWidth();
	if (tex.getHeight() > 0) size *= tex.getHeight();
	if (tex.getDepth() > 0) size *= tex.getDepth();

	return size;
}

}
