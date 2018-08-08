#include "GLObject.h"

namespace renderlib {

GLuint createProgramID() { return glCreateProgram(); }
void deleteProgramID(GLuint id) { glDeleteProgram(id); }

GLuint createVAOID() { GLuint vaoID;  glGenVertexArrays(1, &vaoID); return vaoID; }
void deleteVAOID(GLuint id) { glDeleteVertexArrays(1, &id); }

GLuint createBufferID() { GLuint bufferID; glGenBuffers(1, &bufferID); return bufferID; }
void deleteBufferID(GLuint id) { glDeleteBuffers(1, &id); }

GLuint createTextureID() { GLuint textureID; glGenTextures(1, &textureID); return textureID; }
void deleteTextureID(GLuint id) { glDeleteTextures(1, &id); }

GLuint createFramebufferID() { GLuint framebufferID; glGenFramebuffers(1, &framebufferID); return framebufferID; }
void deleteFramebufferID(GLuint id) { glDeleteFramebuffers(1, &id); }

};