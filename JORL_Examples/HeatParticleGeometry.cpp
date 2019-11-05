#include "HeatParticleGeometry.h"
#include <glSupport.h>

using namespace renderlib;

using namespace glm;

void updateHeatParticleGeometry(HeatParticleGeometry& geom, HeatParticle* particles, size_t particleSize) {
	vec3 quad[4] = {
		vec3(-1.f, 1.f, 0.f),
		vec3(-1.f, -1.f, 0.f),
		vec3(1.f, -1.f, 0.f),
		vec3(1.f, 1.f, 0.f)
	};

	std::vector<vec3> particlePositions;
	std::vector<vec3> particleVelocities;
	std::vector<float> particleHeats;

	for (size_t i = 0; i < particleSize; i++) {
		particlePositions.push_back(particles[i].position);
		particleVelocities.push_back(particles[i].velocity);
		particleHeats.push_back(particles[i].heat);
	}

	geom.loadBuffers(quad, particlePositions.data(), particleVelocities.data(), particleHeats.data(), 4, particleSize);
}


HeatParticle::HeatParticle(glm::vec3 position, glm::vec3 velocity, float heat) 
	:position(position), velocity(velocity), heat(heat)
{}

HeatParticle::HeatParticle() {}