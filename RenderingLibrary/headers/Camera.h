#pragma once

#include <glm/glm.hpp>

class Camera{
protected:
	glm::mat4 projection;

	Camera();
public:
	virtual glm::mat4 getCameraMatrix() const;
	virtual glm::mat4 getProjectionMatrix() const;
	virtual void setProjectionMatrix(const glm::mat4 &projMatrix);
	virtual glm::vec3 getPosition() const;
};