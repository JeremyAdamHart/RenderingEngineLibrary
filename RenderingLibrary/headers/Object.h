#pragma once
#include <glm/gtx/quaternion.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

class Object {
protected:
	Object();
	Object(glm::vec3 position, glm::quat orientation);

	glm::quat orientation;
	glm::vec3 position;
public:
	glm::quat getOrientationQuat() const;
	glm::mat4 getOrientationMat4() const;
	glm::mat4 getTransform() const;
	glm::vec3 getPos() const;
};
