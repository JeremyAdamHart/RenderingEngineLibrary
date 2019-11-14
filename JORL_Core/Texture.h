#pragma once

#include "glSupport.h"
#include "GLObject.h"
#include <vector>

namespace renderlib {

//Never bind a texture to this to draw, only/always use during texture creation
//	Avoids invalidating textures currently bound to a unit
const GLenum NO_ACTIVE_TEXTURE = GL_TEXTURE0;

class TextureManager;

struct TexInfo {
	vector<int> dimensions;
	GLenum target;
	GLint level;
	GLint internalFormat;
	GLenum format;
	GLenum type;

	TexInfo();
	TexInfo(GLenum target, std::vector<int> dimensions, GLint level,
		GLenum format, GLint internalFormat, GLenum type);
};

class Texture {
	GLTexture texID;

	TextureManager *manager;
	int handle;		//Handle for texture manager

public:
	TexInfo info;

	Texture();
	Texture(GLTexture texID, TexInfo info, TextureManager *manager);

	void newTexture(GLuint texID, TexInfo info);
	void deleteTexture();
	void newTexManager(TextureManager *newManager);
	int getTexUnit() const;

	TexInfo& getInfoRef();

	//Access info 
	GLTexture getID() const;
	size_t numDimensions() const;
	int getWidth() const;
	int getHeight() const;
	int getDepth() const;
	GLenum getTarget() const;
	GLint getLevel() const;
	GLenum getFormat() const;
	GLint getInternalFormat() const;
	GLenum getType() const;
};


class TextureManager {
protected:
	TextureManager();

public:
	virtual void removeTexture(int handle);
	virtual int addTexture(Texture tex);	//Adds texture and returns handle

											//Gets texture unit handle is bound to, binding it if necessary
											//Returns NO_ACTIVE_TEXTURE if failed
	virtual int requestTexUnit(int handle);
};

}