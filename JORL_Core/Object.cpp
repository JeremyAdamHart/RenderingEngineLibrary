#include "Object.h"
#include "glmSupport.h"

using namespace glm;

namespace renderlib {

quat Object::getOrientationQuat() const {return orientation;}

vec3 Object::getPos() const {return position;}

mat4 Object::getOrientationMat4() const { return toMat4(orientation); }

mat4 Object::getTransform() const { return translateMatrix(getPos())*getOrientationMat4(); }

Object::Object() :orientation(), position(0.f) {}

Object::Object(vec3 position, quat orientation) : 
	position(position), orientation(orientation) {}

Object::~Object() {}
}
