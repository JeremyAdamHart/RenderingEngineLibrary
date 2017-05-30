#define NOMINMAX
#include "Framebuffer.h"
#include <algorithm>
#include <iterator>
#include <iostream>

const Texture NO_TEXTURE;

Framebuffer::Framebuffer(GLuint id, Texture tex, GLenum attachment) :
id(id), vp(std::max(tex.getWidth(), 0), std::max(tex.getHeight(), 0))
{
	addTexture(tex, attachment);
}

Framebuffer::Framebuffer(unsigned int width, unsigned int height, GLuint id) 
	:id(id), vp(width, height)
{}

bool Framebuffer::addTexture(Texture newTex, GLenum attachment) {
	tex[attachment] = newTex;

	glBindFramebuffer(GL_FRAMEBUFFER, id);
//	glDrawBuffer(GL_NONE);
//	glReadBuffer(GL_NONE);
	glFramebufferTexture(GL_FRAMEBUFFER,
		attachment,
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
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return status;
}
const Texture &Framebuffer::getTexture(GLenum attachment) const { 
	try {
		return tex.at(attachment);
	}
	catch (out_of_range) {
		return NO_TEXTURE;		//Figure out better solution
	}
}

void Framebuffer::use() const {
	glBindFramebuffer(GL_FRAMEBUFFER, id);
	vp.use();
}

GLuint Framebuffer::getID() const { return id; }

Viewport::Viewport(unsigned int width, unsigned int height, 
	unsigned int x, unsigned int y) 
	:width(width), height(height), x(x), y(y) 
{

}

void Framebuffer::deleteTextures() {
	for (map<GLenum, Texture>::iterator it = tex.begin(); it != tex.end(); it++) {
		it->second.deleteTexture();
	}
	tex.clear();
}

void Framebuffer::deleteFramebuffer() {
	glDeleteFramebuffers(1, &id);
}

void Viewport::use() const {
	glViewport(x, y, width, height);
}