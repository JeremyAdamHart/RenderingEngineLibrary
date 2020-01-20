#pragma once

#include <glm/glm.hpp>

namespace renderlib {

class Camera{
public:
	glm::mat4 projection;

public:
	Camera();
	Camera(glm::mat4 projection);
	virtual glm::mat4 getCameraMatrix() const;
	virtual glm::mat4 getRotationMatrix() const;
	virtual glm::mat4 getProjectionMatrix() const;
	virtual void setProjectionMatrix(const glm::mat4 &projMatrix);
	virtual glm::vec3 getPosition() const;
};

}
