#include "Drawable.h"
#include "glSupport.h"
#include <iterator>

namespace renderlib {

using namespace glm;
using namespace std;

Drawable::Drawable(GLGeometryContainer *geometry, Material *material,
	glm::vec3 position, glm::quat orientation) :
	Object(position, orientation), 
	material({ { material->getType(), shared_ptr<Material>(material) } }),
	geometry(geometry),
	scale(1.f)
{}

Drawable::Drawable(GLGeometryContainer *geometry, glm::vec3 position, glm::quat orientation) :
	Object(position, orientation), material({}), geometry(geometry), scale(1.f)
{}

Drawable::Drawable(shared_ptr<GLGeometryContainer> geometry, shared_ptr<Material> material,
	glm::vec3 position, glm::quat orientation) :
	Object(position, orientation),
	material({ { material->getType(), material } }),
	geometry(geometry),
	scale(1.f)
{}

Drawable::Drawable(shared_ptr<GLGeometryContainer> geometry, glm::vec3 position, glm::quat orientation) :
	Object(position, orientation), material({}), geometry(geometry), scale(1.f)
{}


Drawable::Drawable(vec3 position, quat orientation) :Object(position, orientation),
material({}), geometry(nullptr), scale(1.f) {}

shared_ptr<Material> Drawable::getMaterial(int type) {
	try {
		return material.at(type);
	}
	catch (out_of_range) {
		return nullptr;
	}
}

void Drawable::addMaterial(Material* newMaterial) {
	material[newMaterial->getType()] = shared_ptr<Material>(newMaterial);
}

void Drawable::addMaterial(sptr<Material> newMaterial) {
	material[newMaterial->getType()] = newMaterial;
}

bool Drawable::removeMaterial(int type) {
	return material.erase(type) > 0;
}

void Drawable::setGeometryContainer(sptr<GLGeometryContainer> newGeometry)
{
	geometry = newGeometry;
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

mat4 Drawable::getTransform() const {
	return Object::getTransform()*
		mat4({ scale.x, 0, 0, 0,
				0, scale.y, 0, 0,
				0, 0, scale.z, 0,
				0, 0, 0, 1 });
}

void Drawable::setPosition(glm::vec3 newPosition) { position = newPosition;}

void Drawable::setOrientation(glm::quat newOrientation) { orientation = newOrientation; }
void Drawable::setScale(glm::vec3 newScale) { scale = newScale; }
}