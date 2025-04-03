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
	void useAtIndex(unsigned int index) const;
};

class Framebuffer {
protected:
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
	virtual void use() const;

	map<GLenum, Texture>::iterator textureBegin();
	map<GLenum, Texture>::iterator textureEnd();

	void resize(int width, int height);
};

class IndexedFramebuffer : public Framebuffer{
	std::vector<Viewport> vps;
public:
	IndexedFramebuffer();
	IndexedFramebuffer(unsigned int width, unsigned int height);
	IndexedFramebuffer(GLFramebuffer id, Texture tex, GLenum attachment);
	unsigned int addViewport(unsigned int width, unsigned int height, unsigned int x, unsigned int y);
	Framebuffer operator[](unsigned int index);

	virtual void use() const override;
	//void use(unsigned int index) const;
};

void blit(const Framebuffer& read, const Framebuffer& write);
void blit(const Framebuffer& read, const Framebuffer& write, int minX, int minY, int maxX, int maxY);

}