#pragma once

#include "Material.h"
#include "GLGeometry.h"
#include <glm/glm.hpp>
#include "Object.h"
#include <map>

namespace renderlib {

class Drawable : public virtual Object{
protected:
	std::map<int, Material*> material;
	GLGeometryContainer *geometry;
	glm::vec3 scale;

public:
	Drawable(Material *material, GLGeometryContainer *geometry,
		glm::vec3 position=glm::vec3(0.f), glm::quat orientation=glm::quat());
	Drawable(GLGeometryContainer *geometry, glm::vec3 position = glm::vec3(0.f), glm::quat orientation = glm::quat());
	Drawable(glm::vec3 position = glm::vec3(0.f), glm::quat orientation = glm::quat());

	bool loadUniforms(int type, GLint *uniformLocations) const;

	virtual glm::mat4 getTransform() const;

	Material *getMaterial(int type);
	GLGeometryContainer *getGeometryPtr(){ return geometry; }

	void setPosition(glm::vec3 position);
	void setOrientation(glm::quat orientation);
	void setScale(glm::vec3 scale);

	void addMaterial(Material* newMaterial);
	bool removeMaterial(int type);

	void setGeometryContainer(GLGeometryContainer* newGeometry) { geometry = newGeometry; }
	GLGeometryContainer &getGeometry() { return *geometry; }

	void deleteMaterialsAndGeometry();
};

}