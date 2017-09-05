#pragma once

#include <glm/glm.hpp>

namespace renderlib {

class Camera{
protected:
	glm::mat4 projection;

public:
	Camera();
	virtual glm::mat4 getCameraMatrix() const;
	virtual glm::mat4 getProjectionMatrix() const;
	virtual void setProjectionMatrix(const glm::mat4 &projMatrix);
	virtual glm::vec3 getPosition() const;
};

}
