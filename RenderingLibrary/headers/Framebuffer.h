#pragma once

#include "glSupport.h"
#include "GLObject.h"
#include "Texture.h"
#include <map>
#include <iterator>

namespace renderlib {

struct Viewport {
	int x, y, width, height;

	Viewport(unsigned int width=0, unsigned int height=0, 
		unsigned int x = 0, unsigned int y = 0);

	void use() const;
};

class Framebuffer {
	map<GLenum, Texture> tex;

	vector<GLenum> drawBuffers;
public:
	GLFramebuffer id;
	Viewport vp;

	Framebuffer();
	Framebuffer(unsigned int width, unsigned int height, GLFramebuffer id=GLFramebuffer::wrap(0));
	Framebuffer(GLFramebuffer id, Texture tex, GLenum attachment);

	GLFramebuffer getID() const;
	const Texture &getTexture(GLenum attachment) const;
	bool addTexture(Texture newTex, GLenum attachment);
	void use() const;

	map<GLenum, Texture>::iterator textureBegin();
	map<GLenum, Texture>::iterator textureEnd();

	void resize(int width, int height);

//	void deleteTextures();
//	void deleteFramebuffer();
};

void blit(Framebuffer read, Framebuffer write);

}