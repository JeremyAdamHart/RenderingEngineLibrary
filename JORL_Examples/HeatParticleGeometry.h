#pragma once

#include <GLGeometry.h>
#include <GLObject.h>
#include <glm/glm.hpp>
#include <vector>
#include <TemplatedGeometry.h>
#include <VertexAttribLayout.h>

struct HeatParticle{
	HeatParticle(glm::vec3 position, glm::vec3 velocity, float heat);
	HeatParticle();
	glm::vec3 position;
	glm::vec3 velocity;
	float heat;
};

namespace renderlib {
namespace attrib {

struct ParticlePosition :public Instanced<Attribute<glm::vec3>, 1> {
	static inline std::string name() { return "ParticlePosition"; }
};

struct ParticleVelocity :public Instanced<Attribute<glm::vec3>, 1> {
	static inline std::string name() { return "ParticleVelocity"; }
};

struct ParticleHeat :public Instanced<Attribute<float>, 1> {
	static inline std::string name() { return "ParticleHeat"; }
};

}
}

using HeatParticleGeometry = renderlib::GeometryT<
	renderlib::attrib::Position, 
	renderlib::attrib::ParticlePosition, 
	renderlib::attrib::ParticleVelocity, 
	renderlib::attrib::ParticleHeat>;

void updateHeatParticleGeometry(HeatParticleGeometry& geom, HeatParticle* particles, size_t particleSize);

/*
//Stores heat, velocity, radius and position
class HeatParticleGeometry : public renderlib::GLGeometryContainer {
	renderlib::GLVAO vao;
	size_t particleNum;
	
	enum { QUAD_POSITION=0, PARTICLE_POSITION, VELOCITY, HEAT, COUNT };
	std::vector<renderlib::GLBuffer> vbo;

	bool initializeVAO();

public:
	HeatParticleGeometry();
	HeatParticleGeometry(HeatParticle *particles, size_t particleSize);

	void loadParticles(HeatParticle *particles, size_t particleSize);

	virtual void bindGeometry() const;

	virtual void drawGeometry() const;
};
*/