#pragma once

#include <glm/glm.hpp>
#include "HeatParticleGeometry.h"
#include <vector>
#include <queue>

const int MAX_PARTICLES = 100000;

struct Disk {
	Disk(float radius, glm::vec3 origin, glm::vec3 normal);

	float radius;
	glm::vec3 origin;
	glm::vec3 normal;

	void generatePerpendicular();
	//Perpendicular vectors lying on disk
	glm::vec3 v1;	
	glm::vec3 v2;
};

class HeatParticleSystem {
public:
	std::vector<HeatParticle> particles;
	std::vector<float> lifespans;
	std::queue<int> emptySpots;

	HeatParticleSystem();

	void addParticleFromDisk(Disk disk, float velocity, float heat, float lifespan,
		float maxDivergenceAngle = 0.f, float timeOffset =0.f);

	void runSimulation(float timeStep);
};
