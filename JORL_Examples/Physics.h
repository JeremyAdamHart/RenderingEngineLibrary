#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "glmSupport.h"
#include "MeshInfoLoader.h"
#include <Object.h>

const float DAMPING_LINEAR = 5000.f;
const float DAMPING_ANGULAR = 5.f;
const glm::vec3 GRAVITY(0, -9.81, 0);

class RigidBody:public renderlib::Object{
public:

	glm::vec3 force;
	glm::vec3 torque;

	glm::vec3 v;
	float mass; 

	glm::vec3 omega;

	glm::mat3 I;
	glm::mat3 Iinv;

//	RigidBody(float mass, MeshInfoLoader *geometry);

	RigidBody(float mass, glm::mat3 inertialTensor);

	//Applied in world space
	void addLinearForceOnly(glm::vec3 f);
	void addForce(glm::vec3 f, glm::vec3 loc);
	void addTorqueOnly(glm::vec3 f, glm::vec3 loc);
	void resolveForces(float dt);

	glm::vec3 modelToWorld(glm::vec4 modelVec);

	void addGravityForces();
	void addDampingForces(float dampingLinear=DAMPING_LINEAR, float dampingAngular=DAMPING_ANGULAR);
};