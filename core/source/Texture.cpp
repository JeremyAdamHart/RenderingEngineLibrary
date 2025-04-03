#include "Texture.h"

namespace renderlib {

using namespace std;

TexInfo::TexInfo() : dimensions(), target(0), level(-1),
internalFormat(-1), format(0), type(0)
{}

TexInfo::TexInfo(GLenum target, std::vector<int> dimensions, GLint level,
	GLenum format, GLint internalFormat, GLenum type) : target(target), dimensions(dimensions),
	level(level), internalFormat(internalFormat), format(format), type(type) {}

Texture::Texture() : texID(GLTexture::wrap(0)), manager(nullptr), handle(-1){}

Texture::Texture(GLTexture texID, TexInfo info, TextureManager *manager) :
	texID(texID), info(info), manager(manager)
{
	if (manager) handle = manager->addTexture(*this);
}

void Texture::deleteTexture() {
	if(texID)
		glDeleteTextures(1, (GLuint*)&texID);
	if (manager)
		manager->removeTexture(handle);
	texID = -1;
	handle = -1;
}

void Texture::newTexture(GLuint newTexID, TexInfo newInfo) {
	deleteTexture();
	texID = newTexID;
	info = newInfo;
	if (manager) handle = manager->addTexture(*this);
}

int Texture::getTexUnit() const {
	return manager->requestTexUnit(handle);
}

size_t Texture::numDimensions() const { return info.dimensions.size(); }
int Texture::getWidth() const {
	if (info.dimensions.size() > 0)
		return info.dimensions[0];
	else
		return -1;
}

int Texture::getHeight() const {
	if (info.dimensions.size() > 1)
		return info.dimensions[1];
	else
		return -1;
}

int Texture::getDepth() const {
	if (info.dimensions.size() > 2)
		return info.dimensions[2];
	else
		return -1;
}

void Texture::newTexManager(TextureManager *newManager) {
	manager = newManager;
	if (manager) manager->addTexture(*this);
}

GLTexture Texture::getID() const { return texID; }
GLenum Texture::getTarget() const { return info.target; }
GLint Texture::getLevel() const { return info.level; }
GLenum Texture::getFormat() const { return info.format; }
GLenum Texture::getType() const { return info.type; }
GLint Texture::getInternalFormat() const { return info.internalFormat; }
TexInfo& Texture::getInfoRef() { return info; }

TextureManager::TextureManager() {}

void TextureManager::removeTexture(int handle) {}
int TextureManager::addTexture(Texture tex) { return -1; }
int TextureManager::requestTexUnit(int handle) { return 0; }

}