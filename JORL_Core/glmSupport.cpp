#include "glmSupport.h"

using namespace glm;

namespace renderlib {

mat4 scaleMatrix(float scale){
	mat4 matrix (scale);
	matrix[3][3] = 1.f;
	return matrix;
}

mat4 translateMatrix(vec3 pos){
	return mat4(vec4(1, 0, 0, 0),
		vec4(0, 1, 0, 0),
		vec4(0, 0, 1, 0),
		vec4(pos, 1));
}

glm::mat4 scaleMatrix(vec3 scale){
	return mat4(
		vec4(scale.x, 0, 0, 0),
		vec4(0, scale.y, 0, 0),
		vec4(0, 0, scale.z, 0),
		vec4(0, 0, 0, 1.f));
}

glm::vec3 toVec3(vec4 v){
	return vec3(v.x, v.y, v.z);
}

glm::vec4 toVec4(vec3 v, float h){
	return vec4(v.x, v.y, v.z, h);
}

glm::mat3 toMat3(mat4 m){
	return mat3(
		toVec3(m[0]),
		toVec3(m[1]),
		toVec3(m[2])
		);
}

glm::mat4 createAspectRatioMatrix(unsigned int width, unsigned int height) {
	mat4 m = mat4(1.f);
	m[0][0] = float(height) / float(width);
	return m;
}

glm::mat3 rotationMatrix3x3(float theta, glm::vec3 u)
{
	float costheta = cos(theta);
	float sintheta = sin(theta);
	return glm::mat3(
		vec3(
			costheta + u.x*u.x*(1 - costheta),
			u.x*u.y*(1-costheta) + u.z*sintheta,
			u.z*u.x*(1-costheta) - u.y*sintheta
		), vec3(
			u.x*u.y*(1-costheta) - u.z*sintheta,
			costheta + u.y*u.y*(1-costheta),
			u.z*u.y*(1-costheta) + u.x*sintheta
		), vec3(
			u.x*u.z*(1-costheta) + u.y*sintheta,
			u.y*u.z*(1-costheta) - u.x*sintheta,
			costheta + u.z*u.z*(1-costheta)));
}

struct project {
	glm::vec3 p;
	project(glm::vec3 p) :p(p) {}
	glm::vec3 onto(glm::vec3 v) { return (dot(p, v) / dot(v, v))*v; }
};

/*
duplicating A = B;

using Axis = glm::vec3;

project(Axis(v), Point(p));

auto a = projectAontoB(v, a)
auto a = project(v).onto(a);
*/
glm::vec3 project(glm::vec3 v, glm::vec3 p) {
	return (dot(p, v) / dot(v, v))*v;
}

glm::vec3 projectToPlane(glm::vec3 normal, glm::vec3 point) {
	return point - project(normal, point);
}

}