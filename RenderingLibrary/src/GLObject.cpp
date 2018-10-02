#include "GLObject.h"

namespace renderlib {

GLProgram createProgramID() { return GLProgram(glCreateProgram()); }
void deleteProgramID(GLuint id) { glDeleteProgram(id); }

GLShader createShaderID(GLenum type) { return GLShader(glCreateShader(type)); }
void deleteShaderID(GLuint id) { glDeleteShader(id); }

GLVAO createVAOID() { GLuint vaoID;  glGenVertexArrays(1, &vaoID); return GLVAO(vaoID); }
void deleteVAOID(GLuint id) { glDeleteVertexArrays(1, &id); }

GLBuffer createBufferID() { GLuint bufferID; glGenBuffers(1, &bufferID); return GLBuffer(bufferID); }
void deleteBufferID(GLuint id) { glDeleteBuffers(1, &id); }

GLTexture createTextureID() { GLuint textureID; glGenTextures(1, &textureID); return GLTexture(textureID); }
void deleteTextureID(GLuint id) { glDeleteTextures(1, &id); }

GLFramebuffer createFramebufferID() { GLuint framebufferID; glGenFramebuffers(1, &framebufferID); return GLFramebuffer(framebufferID); }
void deleteFramebufferID(GLuint id) { glDeleteFramebuffers(1, &id); }

};