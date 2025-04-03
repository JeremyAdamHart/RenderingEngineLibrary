#include <TemplatedGeometry.h>

namespace renderlib {

template<> GLenum toGLenum<int>() { return GL_INT; }
template<> GLenum toGLenum<unsigned int>() { return GL_UNSIGNED_INT; }
template<> GLenum toGLenum<char>() { return GL_BYTE; }
template<> GLenum toGLenum<unsigned char>() { return GL_UNSIGNED_BYTE; }
template<> GLenum toGLenum<short>() { return GL_SHORT; }
template<> GLenum toGLenum<unsigned short>() { return GL_UNSIGNED_SHORT; }

}