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

/*
HeatParticleGeometry::HeatParticleGeometry() :vao(createVAOID()), particleNum(0){
	initializeVAO();
}

bool HeatParticleGeometry::initializeVAO() {

	for (int i = 0; i < COUNT; i++)
		vbo.push_back(createBufferID());

	glBindVertexArray(vao);
	glEnableVertexAttribArray(ATTRIB_LOCATION::POSITION);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[QUAD_POSITION]);
	glVertexAttribPointer(
		ATTRIB_LOCATION::POSITION,			//Attribute
		3,					//# of components
		GL_FLOAT,			//Type
		GL_FALSE,			//Normalized?
		sizeof(vec3),		//Stride
		(void*)0			//Offset
	);

	//Load quad
	vec3 quad[4] = {
		vec3(-1.f, 1.f, 0.f),
		vec3(-1.f, -1.f, 0.f),
		vec3(1.f, -1.f, 0.f),
		vec3(1.f, 1.f, 0.f)
	};
	glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(vec3), quad, GL_STATIC_DRAW);

	//Particle position
	glEnableVertexAttribArray(ATTRIB_LOCATION::POSITION+PARTICLE_POSITION);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[PARTICLE_POSITION]);
	glVertexAttribPointer(
		ATTRIB_LOCATION::POSITION + PARTICLE_POSITION,			//Attribute
		3,					//# of components
		GL_FLOAT,			//Type
		GL_FALSE,			//Normalized?
		sizeof(HeatParticle),		//Stride
		(void*)offsetof(struct HeatParticle, position)		//Offset
	);

	//Particle velocity
	glEnableVertexAttribArray(ATTRIB_LOCATION::POSITION+VELOCITY);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[VELOCITY]);
	glVertexAttribPointer(
		ATTRIB_LOCATION::POSITION + VELOCITY,			//Attribute
		3,					//# of components
		GL_FLOAT,			//Type
		GL_FALSE,			//Normalized?
		sizeof(HeatParticle),		//Stride
		(void*)offsetof(struct HeatParticle, velocity)			//Offset
	);
	
	//Particle heat
	glEnableVertexAttribArray(ATTRIB_LOCATION::POSITION+HEAT);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[HEAT]);
	glVertexAttribPointer(
		ATTRIB_LOCATION::POSITION + HEAT,		//Attribute
		1,					//# of components
		GL_FLOAT,			//Type
		GL_FALSE,			//Normalized?
		sizeof(HeatParticle),		//Stride
		(void*)offsetof(struct HeatParticle, heat)		//Offset
	);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glVertexAttribDivisor(ATTRIB_LOCATION::POSITION + PARTICLE_POSITION, 1);
	glVertexAttribDivisor(ATTRIB_LOCATION::POSITION + VELOCITY, 1);
	glVertexAttribDivisor(ATTRIB_LOCATION::POSITION + HEAT, 1);	

	glBindVertexArray(0);

	return !checkGLErrors("HeatParticleGeometry::initVAO");
}

void HeatParticleGeometry::loadParticles(HeatParticle *particles, size_t particleSize) {
	glBindVertexArray(vao);

	particleNum = particleSize;
	glBindBuffer(GL_ARRAY_BUFFER, vbo[PARTICLE_POSITION]);
	glBufferData(GL_ARRAY_BUFFER, particleNum * sizeof(HeatParticle), 
		particles, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[VELOCITY]);
	glBufferData(GL_ARRAY_BUFFER, particleNum * sizeof(HeatParticle),
		particles, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[HEAT]);
	glBufferData(GL_ARRAY_BUFFER, particleNum * sizeof(HeatParticle),
		particles, GL_DYNAMIC_DRAW);

	glBindVertexArray(0);

	checkGLErrors("HeatParticleGeometry::loadParticles");
}

void HeatParticleGeometry::bindGeometry() const {
	glBindVertexArray(vao);
}

void HeatParticleGeometry::drawGeometry() const {
	bindGeometry();


	glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, particleNum);
}

*/