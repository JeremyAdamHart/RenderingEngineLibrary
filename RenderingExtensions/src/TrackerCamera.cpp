#include "TrackerCamera.h"
#include <limits>
#include <glm/gtc/matrix_transform.hpp>
#include <stdio.h>

using namespace glm;

TrackerCamera::TrackerCamera() : dir(vec3(0, 0, -1)), right(vec3(1, 0, 0)), up(vec3(0, 1, 0)),
pos(vec3(0, 0, 1.f)){}

TrackerCamera::TrackerCamera(vec3 _dir, vec3 _up, vec3 _pos, const mat4 &projection) : dir(_dir), pos(_pos), up(_up), projection(projection){
	right = normalize(cross(dir, up));
	up = normalize(cross(right, dir));
}

mat4 TrackerCamera::getCameraMatrix() const
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
		vec4(-pos, 1));

	return transpose(cameraRotation)*translation;
}

mat4 TrackerCamera::getProjectionMatrix() const
{
	return projection;
}

vec3 TrackerCamera::getPosition() const
{
	return pos;
}

void TrackerCamera::trackGeometryXZ(glm::vec3 *vertices, size_t numVertices, const glm::mat4 &model_matrix)
{
	float bottom_geometry = std::numeric_limits<float>::max();
	float top_geometry = -std::numeric_limits<float>::max();
	float left_geometry = std::numeric_limits<float>::max();
	float right_geometry = -std::numeric_limits<float>::max();
	float ytop_geometry = -std::numeric_limits<float>::max();
	float ybottom_geometry = std::numeric_limits<float>::max();


	//Determine bounds of geometry
	for (int i = 0; i < numVertices; i++){
		vec4 vert = model_matrix*vec4(vertices[i], 1.f);
		if (vert.x > right_geometry)
			right_geometry = vert.x;
		if (vert.x < left_geometry)
			left_geometry = vert.x;
		if (vert.z > top_geometry)
			top_geometry = vert.z;
		if (vert.z < bottom_geometry)
			bottom_geometry = vert.z;
		if (vert.y > ytop_geometry)
			ytop_geometry = vert.y;
		if (vert.y < ybottom_geometry)
			ybottom_geometry = vert.y;
	}

	pos = vec3((left_geometry + right_geometry)*0.5f, ytop_geometry + 1.f, (bottom_geometry + top_geometry)*0.5f);
	dir = vec3(0, -1, 0);
	up = vec3(0, 0, -1);
	right = vec3(1, 0, 0);

	projection = glm::ortho(left_geometry - pos.x, right_geometry - pos.x, 
		bottom_geometry - pos.z, top_geometry - pos.z, 
		0.5f, pos.y - bottom_geometry+1.f);
}