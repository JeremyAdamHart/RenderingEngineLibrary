#pragma once

#include <glm/gtc/constants.hpp>
#include <glmSupport.h>
#include "Camera.h"

namespace renderlib {

class SphericalCamera : public Camera {
public:
	glm::mat4 projection;
	glm::vec3 center;
	float altitudeMax, altitudeMin, radiusMin, radiusMax;

	SphericalCamera();
	SphericalCamera(float azimuth, float altitude, float radius, glm::vec3 center=glm::vec3(0.f));
	SphericalCamera(glm::mat4 projection, float azimuth, float altitude, float radius);

	void up(float radians);
	void right(float radians);	
	void zoom(float factor);

	glm::mat4 getCameraMatrix() const;
	glm::mat4 getProjectionMatrix() const;
	glm::mat4 getRotationMatrix() const;
	glm::vec3 getPosition() const;

	glm::vec3 right() const;
	glm::vec3 up() const;
	float azimuth, altitude, radius;
private:
};

}