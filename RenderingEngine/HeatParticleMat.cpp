#include "HeatParticleMat.h"
#include "glSupport.h"

using namespace renderlib;

const int HeatParticleMat::id = Material::getNextID();

HeatParticleMat::HeatParticleMat(float radius) :radius(radius)
{}

int HeatParticleMat::getType() const { return id; }

void HeatParticleMat::loadUniforms(GLint *locations) const {
	glUniform1f(locations[RADIUS_LOCATION], radius);

	checkGLErrors("HeatParticleMat::loadUniforms");
}
