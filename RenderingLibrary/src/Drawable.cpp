#include "Drawable.h"
#include "glSupport.h"
#include <iterator>

using namespace glm;

Drawable::Drawable(Material *material, GLGeometryContainer *geometry,
	glm::vec3 position, glm::quat orientation) :
	Object(position, orientation), 
	material({ { material->getType(), material } }),
	geometry(geometry)
{}

Drawable::Drawable(GLGeometryContainer *geometry, glm::vec3 position, glm::quat orientation) :
	Object(position, orientation), material({}), geometry(geometry)
{}

Drawable::Drawable(vec3 position, quat orientation) :Object(position, orientation),
material({}), geometry(nullptr) {}

Material *Drawable::getMaterial(int type) {
	try {
		Material *m = material.at(type);
		return m;
	}
	catch (out_of_range) {
		return nullptr;
	}
}

void Drawable::addMaterial(Material* newMaterial) {
	material[newMaterial->getType()] = newMaterial;
}

bool Drawable::removeMaterial(int type) {
	return material.erase(type) > 0;
}

bool Drawable::loadUniforms(int type, GLint *uniformLocations) const {
	try {
		material.at(type)->loadUniforms(uniformLocations);
		return true;
	}
	catch (out_of_range) {
		return false;
	}
}

void Drawable::deleteMaterialsAndGeometry() {
	map<int, Material*>::iterator it;

	for (it = material.begin(); it != material.end(); it++) {
		delete it->second;
	}
	material.clear();

	delete geometry;
}

void Drawable::setPosition(glm::vec3 newPosition) { position = newPosition;}

void Drawable::setOrientation(glm::quat newOrientation) { orientation = newOrientation; }