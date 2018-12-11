#include "HeatParticleSystem.h"
#include <random>
#include <ctime>
#include <cmath>

#define PI 3.14159265359f

const float DAMPING = 0.001f;
const float HEAT_DECAY = 10.f;

using namespace glm;
using namespace renderlib;

float rand01() {
	return float(rand()) / float(RAND_MAX);
}

Disk::Disk(float radius, vec3 origin, vec3 normal) :
	radius(radius), origin(origin), normal(normal) {
	generatePerpendicular();
}

void Disk::generatePerpendicular() {
	vec3 p = cross(normal, vec3(0, 1, 0));
	if (length(p) < 0.001f)
		p = normalize(cross(normal, vec3(1, 0, 0)));
	else
		p = normalize(p);

	v1 = p;
	v2 = cross(v1, normal);
}

HeatParticleSystem::HeatParticleSystem() {
	srand(time(0));
}

void HeatParticleSystem::addParticleFromDisk(Disk disk, float velocity, float heat, 
	float lifespan, float maxDivergenceAngle, float timeOffset) {

	if (particles.size() > MAX_PARTICLES)
		return;

	float r = rand01()*disk.radius;
	float theta = rand01()*2.f*PI;
	vec3 position = disk.v1*sqrt(r)*cos(theta) + disk.v2*sqrt(r)*sin(theta)
		+disk.origin;
	HeatParticle newParticle;
	if (maxDivergenceAngle < 0.0001f) {
		newParticle = HeatParticle(position, velocity*disk.normal, heat);
	}
	else {
		float cos_phi = cosf(maxDivergenceAngle);
		float z = cos_phi + (1 - cos_phi)*rand01();
		theta = rand01()*2.f*PI;
		vec3 pVelocity = sqrt(1 - z*z)*cosf(theta)*disk.v1+
			sqrt(1 - z*z)*sinf(theta)*disk.v2+
			z*disk.normal;
		newParticle = HeatParticle(position, velocity*pVelocity, heat);
	}

	newParticle.position += newParticle.velocity*timeOffset;

	if (emptySpots.size() == 0) {
		particles.push_back(newParticle);
		lifespans.push_back(lifespan);
	}
	else {
		particles[emptySpots.front()] = newParticle;
		lifespans[emptySpots.front()] = lifespan;
		emptySpots.pop();
	}
}

void HeatParticleSystem::runSimulation(float timeStep) {
	for (int i = 0; i < particles.size(); i++) {
		if (lifespans[i] >= 0.f) {
			lifespans[i] -= timeStep;
			if (lifespans[i] < 0.f) {
				particles[i].heat = 0.f;
				emptySpots.push(i);		//Queue for replacement
			}

			particles[i].velocity -= particles[i].velocity*DAMPING*timeStep;
			particles[i].position += particles[i].velocity*timeStep;
			particles[i].heat *= 1- HEAT_DECAY*timeStep;
		}
	}
}
