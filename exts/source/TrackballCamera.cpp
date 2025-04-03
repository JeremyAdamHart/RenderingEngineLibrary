#include "TrackballCamera.h"
#include <cstdio>

namespace renderlib {

mat4 rotateAbout(vec3 axis, float radians)
{
	mat4 matrix;

	matrix[0][0] = cos(radians) + axis.x*axis.x*(1 - cos(radians));
	matrix[1][0] = axis.x*axis.y*(1 - cos(radians)) - axis.z*sin(radians);
	matrix[2][0] = axis.x*axis.z*(1 - cos(radians)) + axis.y*sin(radians);

	matrix[0][1] = axis.y*axis.x*(1 - cos(radians)) + axis.z*sin(radians);
	matrix[1][1] = cos(radians) + axis.y*axis.y*(1 - cos(radians));
	matrix[2][1] = axis.y*axis.z*(1 - cos(radians)) - axis.x*sin(radians);

	matrix[0][2] = axis.z*axis.x*(1 - cos(radians)) - axis.y*sin(radians);
	matrix[1][2] = axis.z*axis.y*(1 - cos(radians)) + axis.x*sin(radians);
	matrix[2][2] = cos(radians) + axis.z*axis.z*(1 - cos(radians));

	return matrix;
}

TrackballCamera::TrackballCamera() : dir(vec3(0, 0, -1)), right(vec3(1, 0, 0)), up(vec3(0, 1, 0)),
pos(vec3(0, 0, 1.f)), center(vec3(0.f)){}

TrackballCamera::TrackballCamera(vec3 _dir, vec3 _pos, const mat4 &projection) :dir(_dir), pos(_pos), center(vec3(0.f)), projection(projection){
	right = normalize(cross(dir, vec3(0, 1, 0)));
	up = normalize(cross(right, dir));
}

void TrackballCamera::trackballUp(float radians)
{
	mat4 rotation = rotateAbout(right, -radians);

	vec4 newPos = rotation*vec4(pos, 1);
	pos = vec3(newPos.x, newPos.y, newPos.z);

	vec4 newUp = rotation*vec4(up, 1);
	up = normalize(vec3(newUp.x, newUp.y, newUp.z));

	vec4 newDir = rotation*vec4(dir, 1);
	dir = normalize(vec3(newDir.x, newDir.y, newDir.z));

	printf("Position (%f, %f, %f)\n", getPosition().x, getPosition().y, getPosition().z);
	printf("Center (%f, %f, %f)\n", center.x, center.y, center.z);
	mat4 m = getProjectionMatrix();
	printf("Projection matrix = \n| %.2f %.2f %.2f %.2f |\n| %.2f %.2f %.2f %.2f |\n| %.2f %.2f %.2f %.2f |\n| %.2f %.2f %.2f %.2f |\n",
		m[0][0], m[1][0], m[2][0], m[3][0],
		m[0][1], m[1][1], m[2][1], m[3][1],
		m[0][2], m[1][2], m[2][2], m[3][2],
		m[0][3], m[1][3], m[2][3], m[3][3]
	);

	m = getRotationMatrix();
	printf("Rotation matrix = \n| %.2f %.2f %.2f %.2f |\n| %.2f %.2f %.2f %.2f |\n| %.2f %.2f %.2f %.2f |\n| %.2f %.2f %.2f %.2f |\n",
		m[0][0], m[1][0], m[2][0], m[3][0],
		m[0][1], m[1][1], m[2][1], m[3][1],
		m[0][2], m[1][2], m[2][2], m[3][2],
		m[0][3], m[1][3], m[2][3], m[3][3]
	);

	m = getCameraMatrix();
	printf("Camera matrix = \n| %.2f %.2f %.2f %.2f |\n| %.2f %.2f %.2f %.2f |\n| %.2f %.2f %.2f %.2f |\n| %.2f %.2f %.2f %.2f |\n",
		m[0][0], m[1][0], m[2][0], m[3][0],
		m[0][1], m[1][1], m[2][1], m[3][1],
		m[0][2], m[1][2], m[2][2], m[3][2],
		m[0][3], m[1][3], m[2][3], m[3][3]
	);
}

void TrackballCamera::trackballRight(float radians)
{
	mat4 rotation = rotateAbout(up, radians);

	vec4 newPos = rotation*vec4(pos, 1);
	pos = vec3(newPos.x, newPos.y, newPos.z);

	vec4 newRight = rotation*vec4(right, 1);
	right = normalize(vec3(newRight.x, newRight.y, newRight.z));

	vec4 newDir = rotation*vec4(dir, 1);
	dir = normalize(vec3(newDir.x, newDir.y, newDir.z));
}

void TrackballCamera::zoom(float factor)
{
	pos = -dir*length(pos)*factor;
}

mat4 TrackballCamera::getCameraMatrix() const
{
	mat4 cameraRotation = mat4(
		vec4(right, 0),
		vec4(up, 0),
		vec4(-dir, 0),
		vec4(0, 0, 0, 1));

	mat4 translation = mat4(
		vec4(1, 0, 0, 0),
		vec4(0, 1, 0, 0),
		vec4(0, 0, 1, 0),
		vec4(-(pos + center), 1));

	return transpose(cameraRotation)*translation;
}

mat4 TrackballCamera::getRotationMatrix() const
{
	return mat4(
		vec4(right, 0),
		vec4(up, 0),
		vec4(-dir, 0),
		vec4(0, 0, 0, 1));
}

mat4 TrackballCamera::getProjectionMatrix() const
{
	return projection;
}

vec3 TrackballCamera::getPosition() const
{
	return pos+center;
}

}