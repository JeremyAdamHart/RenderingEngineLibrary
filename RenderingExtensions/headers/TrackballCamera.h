#pragma once

#include "glm/glm.hpp"
#include "Camera.h"

using namespace glm;

class TrackballCamera : public Camera{
public:
	vec3 dir;
	vec3 right;
	vec3 up;


	vec3 center;
	vec3 pos;

	mat4 projection;

	TrackballCamera();

	TrackballCamera(vec3 _dir, vec3 _pos, const mat4 &projection=mat4());

	void setFieldOfView(float fov);

	void trackballUp(float radians);
	void trackballRight(float radians);
	void zoom(float factor);
	mat4 getCameraMatrix() const;
	mat4 getProjectionMatrix() const;
	vec3 getPosition() const;
};