#include <glm/glm.hpp>

glm::mat4 scaleMatrix(float scale);
glm::mat4 scaleMatrix(glm::vec3 scale);
glm::mat4 translateMatrix(glm::vec3 pos);
glm::vec3 toVec3(glm::vec4 v);
glm::vec4 toVec4(glm::vec3 v, float h);
glm::mat3 toMat3(glm::mat4 m);