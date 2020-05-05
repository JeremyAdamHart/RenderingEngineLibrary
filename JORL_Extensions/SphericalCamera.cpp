#include "SphericalCamera.h"
#include <iostream>

namespace renderlib {

using namespace glm;

const float DEFAULT_ALTITUDE_MAX = 0.95f*0.5f*glm::pi<float>();
const float DEFAULT_ALTITUDE_MIN = -0.95f*0.5f*glm::pi<float>();
const float DEFAULT_RADIUS_MAX = 10000.f;
const float DEFAULT_RADIUS_MIN = 0.1f;


SphericalCamera::SphericalCamera():
	projection(mat4(1.f)), azimuth(0.f), altitude(0.f), radius(1.f), center(glm::vec3(0.f)),
	altitudeMax(DEFAULT_ALTITUDE_MAX), altitudeMin(DEFAULT_ALTITUDE_MIN), radiusMax(DEFAULT_RADIUS_MAX), radiusMin(DEFAULT_RADIUS_MIN)
{}

SphericalCamera::SphericalCamera(float azimuth, float altitude, float radius, vec3 center):
	projection(mat4(1.f)), azimuth(azimuth), altitude(altitude), radius(radius), center(center),
	altitudeMax(DEFAULT_ALTITUDE_MAX), altitudeMin(DEFAULT_ALTITUDE_MIN), radiusMax(DEFAULT_RADIUS_MAX), radiusMin(DEFAULT_RADIUS_MIN)
{}

SphericalCamera::SphericalCamera(glm::mat4 projection, float azimuth, float altitude, float radius) :
	projection(projection), azimuth(azimuth), altitude(altitude), center(center), radius(radius),
	altitudeMax(DEFAULT_ALTITUDE_MAX), altitudeMin(DEFAULT_ALTITUDE_MIN), radiusMax(DEFAULT_RADIUS_MAX), radiusMin(DEFAULT_RADIUS_MIN)
{}

void SphericalCamera::up(float radians) {
	altitude = clamp(altitude + radians, altitudeMin, altitudeMax);
}

void SphericalCamera::right(float radians){
	azimuth += radians;
}

void SphericalCamera::zoom(float factor){
	radius = clamp(radius*factor, radiusMin, radiusMax);
}

mat4 SphericalCamera::getCameraMatrix() const {
	return transpose(getRotationMatrix())*translateMatrix(-getPosition());
}

mat4 SphericalCamera::getProjectionMatrix() const {
	return projection;
}

mat4 SphericalCamera::getRotationMatrix() const {
	glm::vec3 z = normalize(getPosition() - center);
	glm::vec3 x = normalize(cross(vec3(0, 1, 0), z));
	glm::vec3 y = normalize(cross(z, x));
	glm::vec3 up = normalize(cross(z, right()));
	//printf("z (%f %f %f)\n", z.x, z.y, z.z);
	/*return glm::transpose(mat4(
		x.x, x.y, x.z, 0,
		y.x, y.y, y.z, 0,
		z.x, z.y, z.z, 0,
		0, 0, 0, 1));
		*/
	return mat4(
		vec4(x, 0),
		vec4(y, 0),
		vec4(z, 0),
		vec4(0, 0, 0, 1)
	);
	/*
	return glm::transpose(mat4(
		cos(azimuth), 0, -sin(azimuth), 0,
		sin(altitude)*sin(azimuth), cos(altitude), sin(altitude)*cos(azimuth), 0,
		cos(altitude)*sin(azimuth), sin(altitude), cos(altitude)*cos(azimuth), 0,
		0, 0, 0, 1
	));*/
}

vec3 SphericalCamera::getPosition() const {
	return center + radius*glm::vec3(cos(altitude)*sin(azimuth), sin(altitude), cos(altitude)*cos(azimuth));
}

glm::vec3 SphericalCamera::right() const
{
	return glm::vec3(cos(azimuth), 0, -sin(azimuth));
}

glm::vec3 SphericalCamera::up() const
{
	glm::vec3 z = normalize(getPosition() - center);
	glm::vec3 x = normalize(cross(vec3(0, 1, 0), z));
	glm::vec3 y = normalize(cross(z, x));
	glm::vec3 up = normalize(cross(z, right()));
	return up;	// glm::vec3(sin(altitude)*sin(azimuth), cos(altitude), sin(altitude)*cos(azimuth));
}


}
