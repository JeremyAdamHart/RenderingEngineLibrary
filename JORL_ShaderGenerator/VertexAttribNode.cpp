#include <VertexAttribNode.h>

namespace node {

template<>
size_t attributeSize<glm::vec2>() { return 1; }

template<>
size_t attributeSize<glm::vec3>() { return 1; }

template<>
size_t attributeSize<vec4>() { return 1; }

template<>
size_t attributeSize<float>() { return 1; }

template<>
size_t attributeSize<glm::mat2>() { return 2; }

template<>
size_t attributeSize<glm::mat3>() { return 3; }

template<>
size_t attributeSize<glm::mat4>() { return 4; }

template<>
size_t attributeSize<glm::mat3>() { return 3; }

template<>
size_t attributeSize<unsigned char>() { return 1; }

template<>
size_t attributeSize<int>() { return 1; }
}