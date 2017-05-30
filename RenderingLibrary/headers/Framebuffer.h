#pragma once

#include "glSupport.h"
#include "Texture.h"
#include <map>

struct Viewport {
	int x, y, width, height;

	Viewport(unsigned int width=0, unsigned int height=0, 
		unsigned int x = 0, unsigned int y = 0);

	void use() const;
};

class Framebuffer {
	map<GLenum, Texture> tex;
public:
	GLuint id;
	Viewport vp;

	Framebuffer(unsigned int width, unsigned int height, GLuint id=0);
	Framebuffer(GLuint id, Texture tex, GLenum attachment);

	GLuint getID() const;
	const Texture &getTexture(GLenum attachment) const;
	bool addTexture(Texture newTex, GLenum attachment);
	void use() const;

	void deleteTextures();
	void deleteFramebuffer();
};