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

glm::vec3 project(glm::vec3 v, glm::vec3 p) {
	return (dot(p, v) / dot(v, v))*v;
}

glm::vec3 projectToPlane(glm::vec3 normal, glm::vec3 point) {
	return point - project(normal, point);
}

}