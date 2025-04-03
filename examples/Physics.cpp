#include "Physics.h"
#include <stdio.h>
#include <random>
#include <ctime>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;
using namespace renderlib;

RigidBody::RigidBody(float mass, mat3 inertialTensor) :Object(vec3(0.f), quat()), force(0.f), torque(0.f), v(0.f), 
	mass(mass), omega(0.f), I(inertialTensor), Iinv(inverse(inertialTensor))
{

}

void RigidBody::addLinearForceOnly(glm::vec3 f) {
	force += f;
}

void RigidBody::addForce(vec3 f, vec3 loc){
	vec3 r = loc - position;
	force += f;
	torque += cross(r, f);
}

void RigidBody::addTorqueOnly(vec3 f, vec3 loc) {
	vec3 r = loc - position;
	torque += cross(r, f);
}

void RigidBody::resolveForces(float dt){

	//Linear integration
	v += (force/mass)*dt;
	position += v*dt;

	//Rotational integration - Body space to world space
	mat3 IinvWorld = mat3_cast(orientation)*Iinv*transpose(mat3_cast(orientation));
	vec3 ddt_omega = torque;		//World space
	omega += ddt_omega*dt;					//World space
	vec3 angVelocityV = IinvWorld*omega;	//World space
	quat angVelocityQ (0, angVelocityV.x, angVelocityV.y, angVelocityV.z);

	orientation += dt*	0.5f*angVelocityQ*orientation;
	orientation = normalize(orientation);

	force = vec3(0.0);
	torque = vec3(0.0);
}

static float rand01() { return float(rand()) / float(RAND_MAX); }

void RigidBody::addGravityForces() {
	force += GRAVITY*mass;
}

void RigidBody::addDampingForces(float dampingLinear, float dampingAngular) {
	force += -v*dampingLinear;
	torque += -omega*dampingAngular;
}

vec3 RigidBody::modelToWorld(vec4 modelVec) {
	vec4 worldVec = (getTransform())*modelVec;
	return vec3(worldVec.x, worldVec.y, worldVec.z);
}

//-0.22352  1.26375