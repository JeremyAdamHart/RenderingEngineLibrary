#pragma once

#include <GLGeometry.h>
#include <glm/glm.hpp>

namespace renderlib {

struct HeatParticle{
	HeatParticle(glm::vec3 position, glm::vec3 velocity, float heat);
	HeatParticle();
	glm::vec3 position;
	glm::vec3 velocity;
	float heat;
};

//Stores heat, velocity, radius and position
class HeatParticleGeometry : public GLGeometryContainer {
	GLuint vao;
	size_t particleNum;
	
	enum { QUAD_POSITION=0, PARTICLE_POSITION, VELOCITY, HEAT, COUNT };
	GLuint vbo[COUNT];

	bool initializeVAO();

public:
	HeatParticleGeometry();
	HeatParticleGeometry(HeatParticle *particles, size_t particleSize);

	void loadParticles(HeatParticle *particles, size_t particleSize);

	virtual void bindGeometry() const;

	virtual void drawGeometry() const;
};

}