#pragma once

#include "glmSupport.h"
#include <glm/gtc/quaternion.hpp>

struct PetioleInfo {
	glm::vec3 start, middle, end;
	glm::quat endOrientation;
};

PetioleInfo petioleRotation(glm::vec3 location, glm::vec3 tangent, glm::vec3 normal, float segmentLength, float targetAngleFromVertical, float maxBendAngle, float maxTwistAngle);