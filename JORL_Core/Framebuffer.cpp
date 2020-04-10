#define NOMINMAX
#include "Framebuffer.h"
#include <algorithm>
#include <iterator>
#include <iostream>

namespace renderlib {

Framebuffer::Framebuffer() : id(GLFramebuffer::wrap(0)), vp(0, 0, 0, 0) {

}

Framebuffer::Framebuffer(GLFramebuffer id, Texture tex, GLenum attachment) :
	id(id), vp(std::max(tex.getWidth(), 0), std::max(tex.getHeight(), 0))
{
	addTexture(tex, attachment);
}

Framebuffer::Framebuffer(unsigned int width, unsigned int height, GLFramebuffer id)
	:id(id), vp(width, height)
{}

bool Framebuffer::addTexture(Texture newTex, GLenum attachment) {
	tex[attachment] = newTex;


	glBindTexture(newTex.getTarget(), newTex.getID());
	glBindFramebuffer(GL_FRAMEBUFFER, id);
	glFramebufferTexture2D(GL_FRAMEBUFFER,
		attachment,
		newTex.getTarget(),
		newTex.getID(),
		newTex.getLevel());


	bool status = true;

	GLenum FBO_STATUS = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (FBO_STATUS != GL_FRAMEBUFFER_COMPLETE) {
		status = false;

		switch (FBO_STATUS) {
		case GL_FRAMEBUFFER_UNDEFINED:
			std::cout << "GL_FRAMEBUFFER_UNDEFINED" << std::endl;
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
			std::cout << "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT" << std::endl;
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
			std::cout << "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT" << std::endl;
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
			std::cout << "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER" << std::endl;
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
			std::cout << "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER" << std::endl;
			break;
		case GL_FRAMEBUFFER_UNSUPPORTED:
			std::cout << "GL_FRAMEBUFFER_UNSUPPORTED" << std::endl;
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
			std::cout << "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE" << std::endl;
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
			std::cout << "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS" << std::endl;
			break;
		}
	}
	if (status && attachment != GL_DEPTH_ATTACHMENT) {
		drawBuffers.push_back(attachment);
		glDrawBuffers(drawBuffers.size(), &drawBuffers[0]);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	return status;
}
const Texture &Framebuffer::getTexture(GLenum attachment) const {
	try {
		return tex.at(attachment);
	}
	catch (out_of_range) {
		std::cout << "Texture does not exist" << std::endl;		//Figure out better solution
	}
}

void Framebuffer::use() const {
	glBindFramebuffer(GL_FRAMEBUFFER, id);
	if (drawBuffers.size() != 0)
		glDrawBuffers(drawBuffers.size(), &drawBuffers[0]);
	vp.use();
}

GLFramebuffer Framebuffer::getID() const { return id; }

void Framebuffer::resize(int width, int height) {
	for (auto it = tex.begin(); it != tex.end(); it++) {
		GLenum attachment = it->first;
		Texture tex = it->second;
		TexInfo info = it->second.getInfoRef();

		if (info.target == GL_TEXTURE_2D) {
			info.dimensions[0] = width;
			info.dimensions[1] = height;
			glActiveTexture(NO_ACTIVE_TEXTURE);
			glBindTexture(info.target, tex.getID());
			glTexImage2D(info.target, info.level, info.internalFormat, info.dimensions[0], info.dimensions[1],
				0, info.format, GL_FLOAT, nullptr);
			glBindTexture(GL_TEXTURE_2D, 0);

			glBindFramebuffer(GL_FRAMEBUFFER, id);
			glFramebufferTexture2D(GL_FRAMEBUFFER,
				attachment,
				tex.getTarget(),
				tex.getID(),
				tex.getLevel());
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

	}

	vp.width = width;
	vp.height = height;
}

Viewport::Viewport(unsigned int width, unsigned int height,
	unsigned int x, unsigned int y)
	:width(width), height(height), x(x), y(y)
{

}

IndexedFramebuffer::IndexedFramebuffer() : Framebuffer(){}

IndexedFramebuffer::IndexedFramebuffer(unsigned int width, unsigned int height)
	: Framebuffer(width, height, createFramebufferID()) {}
IndexedFramebuffer::IndexedFramebuffer(GLFramebuffer id, Texture tex, GLenum attachment)
	: Framebuffer(id, tex, attachment) {}

unsigned int IndexedFramebuffer::addViewport(unsigned int width, unsigned int height, unsigned int x, unsigned int y) {
	vps.push_back(Viewport(width, height, x, y));

	return vps.size() - 1;
}

Framebuffer IndexedFramebuffer::operator[](unsigned int index) {
	Framebuffer ret(vp.width, vp.height, id);
	ret.vp = vps[index];
	return ret;
}


void IndexedFramebuffer::use() const {
	glBindFramebuffer(GL_FRAMEBUFFER, id);
	if (drawBuffers.size() != 0)
		glDrawBuffers(drawBuffers.size(), &drawBuffers[0]);
	for (int i = 0; i < vps.size(); i++) {
		glViewportIndexedf(i, vps[i].x, vps[i].y, vps[i].width, vps[i].height);
	}
	if (!vps.size())
		glViewport(vp.x, vp.y, vp.width, vp.height);
}

/*
void Framebuffer::deleteTextures() {
	for (map<GLenum, Texture>::iterator it = tex.begin(); it != tex.end(); it++) {
		it->second.deleteTexture();
	}
	tex.clear();
}

void Framebuffer::deleteFramebuffer() {
	glDeleteFramebuffers(1, &id);
}
*/
void Viewport::use() const {
	glViewport(x, y, width, height);
}

void Viewport::useAtIndex(unsigned int index) const {
	glViewportIndexedf(index, x, y, width, height);
}

map<GLenum, Texture>::iterator Framebuffer::textureBegin() {
	return tex.begin();
}

map<GLenum, Texture>::iterator Framebuffer::textureEnd() {
	return tex.end();
}

void blit(const Framebuffer& read, const Framebuffer& write, int minX, int minY, int maxX, int maxY) {
	glBindFramebuffer(GL_READ_FRAMEBUFFER, read.getID());
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, write.getID());

	glBlitFramebuffer(minX, minY, maxX, maxY,
		minX, minY, maxX, maxY,
		GL_COLOR_BUFFER_BIT, GL_LINEAR);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void blit(const Framebuffer& read, const Framebuffer& write) {
	glBindFramebuffer(GL_READ_FRAMEBUFFER, read.getID());
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, write.getID());

	glBlitFramebuffer(0, 0, read.vp.width, read.vp.height,
		0, 0, write.vp.width, write.vp.height,
		GL_COLOR_BUFFER_BIT, GL_LINEAR);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);	
}

}