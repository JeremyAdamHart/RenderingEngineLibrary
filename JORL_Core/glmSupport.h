#include <glm/glm.hpp>

namespace renderlib {

glm::mat4 scaleMatrix(float scale);
glm::mat4 scaleMatrix(glm::vec3 scale);
glm::mat4 translateMatrix(glm::vec3 pos);
glm::vec3 toVec3(glm::vec4 v);
glm::vec4 toVec4(glm::vec3 v, float h);
glm::mat3 toMat3(glm::mat4 m);
glm::vec3 project(glm::vec3 v, glm::vec3 p);	//Projects p onto v
glm::vec3 projectToPlane(glm::vec3 normal, glm::vec3 point);
glm::mat4 createAspectRatioMatrix(unsigned int width, unsigned int height);
glm::mat3 rotationMatrix3x3(float angle, glm::vec3 axis);

}